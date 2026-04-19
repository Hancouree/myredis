#include <iostream>
#include <unordered_map>
#include <queue>
#include <optional>
#include <boost/asio.hpp>

namespace asio = boost::asio;
using tcp = boost::asio::ip::tcp;

class Parser {
public:
    bool parse(boost::asio::streambuf& buf, std::vector<std::string>& out_args) {
        if (buf.size() == 0) return false;
        
        std::istream is(&buf);

        try
        {
            char type;
            if (!(is >> type)) return false;

            if (type != '*') return false;

            int num_elements;
            if (!(is >> num_elements)) return false;
            is.ignore(2);

            for (int i = 0; i < num_elements; ++i) {
                char data_type;
                if (!(is >> data_type)) return false;

                int length;
                if (!(is >> length)) return false;
                is.ignore(2);

                std::string data(length, ' ');
                is.read(&data[0], length);
                is.ignore(2);

                out_args.push_back(data);
            }

            return true;
        }
        catch (const std::exception&)
        {
            return false;
        }
    }
};

struct Record {
    std::string value;
    std::optional<std::chrono::steady_clock::time_point> expires_at;
};

class Repository {
public:
    void set(const std::string& key, const std::string& value) {
        m_data[key] = { value, std::nullopt };
    }

    bool expires(const std::string& key, int seconds) {
        if (auto it = m_data.find(key); it != m_data.end()) {
            it->second.expires_at = std::chrono::steady_clock::now() + std::chrono::seconds(seconds);
            return true;
        }

        return false;
    }

    std::string get(const std::string& key) {
        if (auto it = m_data.find(key); it != m_data.end()) {
            auto now = std::chrono::steady_clock::now();
            if (it->second.expires_at.has_value()) {
                if (now >= it->second.expires_at) {
                    m_data.erase(it);
                    return "";
                }
            }

            return it->second.value;
        }

        return "";
    }

    bool del(const std::string& key) {
        return m_data.erase(key) > 0;
    }
private:
    std::unordered_map<std::string, Record> m_data;
};

class Handler {
public:
    virtual ~Handler() = default;
    virtual void execute(const std::vector<std::string>& args, std::shared_ptr<Repository>& m_repo, std::function<void(const std::string&)> callback) = 0;
};

class PingHandler : public Handler {
public:
    void execute(const std::vector<std::string>& args, std::shared_ptr<Repository>& m_repo, std::function<void(const std::string&)> callback) override {
        callback("PONG\r\n");
    }
};

class SetHandler : public Handler {
public:
    void execute(const std::vector<std::string>& args, std::shared_ptr<Repository>& m_repo, std::function<void(const std::string&)> callback) override {
        if (args.size() < 3) {
            callback("-ERR wrong number of arguments for SET\r\n");
            return;
        }

        m_repo->set(args[1], args[2]);
        callback("+OK\r\n");
    }
};

class GetHandler : public Handler {
public:
    void execute(const std::vector<std::string>& args, std::shared_ptr<Repository>& m_repo, std::function<void(const std::string&)> callback) override {
        if (args.size() < 2) {
            callback("-ERR wrong number of arguments for GET\r\n");
            return;
        }

        std::string val = m_repo->get(args[1]);
        if (val.empty()) {
            callback("$-1\r\n");
        }
        else {
            callback("$" + std::to_string(val.size()) + "\r\n" + val + "\r\n");
        }
    }
};

class ExpireHandler : public Handler {
public:
    void execute(const std::vector<std::string>& args, std::shared_ptr<Repository>& m_repo, std::function<void(const std::string&)> callback) override {
        if (args.size() < 3) {
            callback("-ERR wrong number of arguments for EXPIRE\r\n");
            return;
        }

        try
        {
            int seconds = std::stoi(args[2]);

            if (m_repo->expires(args[1], seconds)) {
                callback(":1\r\n");
            }
            else {
                callback(":0\r\n");
            }
        }
        catch (const std::exception&)
        {
            callback("-ERR value is not an integer or out of range\r\n");
        }
    }
};

class DelHandler : public Handler {
public:
    void execute(const std::vector<std::string>& args, std::shared_ptr<Repository>& m_repo, std::function<void(const std::string&)> callback) override {
        if (args.size() < 2) {
            callback("-ERR wrong number of arguments for DEL\r\n");
            return;
        }

        if (m_repo->del(args[1])) {
            callback(":1\r\n");
        }
        else {
            callback(":0\r\n");
        }
    }
};

class Registry {
public:
    Registry() = delete;

    static void init() {
        m_handlers["PING"] = std::make_shared<PingHandler>();
        m_handlers["SET"] = std::make_shared<SetHandler>();
        m_handlers["GET"] = std::make_shared<GetHandler>();
        m_handlers["EXPIRE"] = std::make_shared<ExpireHandler>();
        m_handlers["DEL"] = std::make_shared<DelHandler>();
    }

    static void handle(
        const std::string& cmd, 
        const std::vector<std::string>& args,
        std::shared_ptr<Repository>& m_repo,
        std::function<void(const std::string&)> callback
    ) {
        if (auto it = m_handlers.find(cmd); it != m_handlers.end()) {
            it->second->execute(args, m_repo, callback);
        }
    }
private:
    static std::unordered_map<std::string, std::shared_ptr<Handler>> m_handlers;
};

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(tcp::socket s, std::shared_ptr<Repository> repo) 
        : m_socket(std::move(s))
        , m_repo(repo) 
    {}

    void run() { doRead(); }
private:
    void doRead() {
        m_socket.async_read_some(m_buffer.prepare(1024),
            [self = shared_from_this()](boost::system::error_code ec, size_t received) {
                if (!ec) {
                    self->m_buffer.commit(received);

                    std::vector<std::string> args;
                    while (self->m_parser.parse(self->m_buffer, args)) {
                        self->handleCommand(args);
                        args.clear();
                    }

                    self->doRead();
                }
                else std::cout << ec.message() << "\n";
            });
    }

    void doWrite(const std::string& msg) {
        bool in_progress = !m_writingQueue.empty();
        m_writingQueue.push(msg);

        if (!in_progress) {
            doWriteNext();
        }
    }

    void doWriteNext() {
        auto self(shared_from_this());
        auto buf = std::make_shared<std::string>(m_writingQueue.front());

        asio::async_write(m_socket, asio::buffer(*buf),
            [this, self, buf](boost::system::error_code ec, size_t) {
                if (!ec) {
                    m_writingQueue.pop();
                    if (!m_writingQueue.empty()) {
                        doWriteNext();
                    }
                }
            });
    }

    void handleCommand(std::vector<std::string>& args) {
        if (args.empty()) return;

        Registry::handle(
            args[1], 
            args,
            m_repo,
            [this](const std::string& answer) { doWrite(answer); }
        );
    }

    tcp::socket m_socket;
    asio::streambuf m_buffer;
    Parser m_parser;
    std::shared_ptr<Repository> m_repo;
    std::queue<std::string> m_writingQueue;
};

class Listener : public std::enable_shared_from_this<Listener> {
public:
    Listener(asio::io_context& ctx) 
        : m_acceptor(ctx, { boost::asio::ip::make_address("127.0.0.1"), 5050 }) 
        , m_repo(std::make_shared<Repository>())
    {}

    void run() { doAccept(); }
private:
    void doAccept() {
        m_acceptor.async_accept(
            [self = shared_from_this()](boost::system::error_code ec, tcp::socket s) {
                if (!ec) {
                    std::make_shared<Session>(std::move(s), self->m_repo)->run();
                } 

                self->doAccept();
        });
    }

    tcp::acceptor m_acceptor;
    std::shared_ptr<Repository> m_repo;
};

int main()
{
    Registry::init();

    asio::io_context ctx;

    std::make_shared<Listener>(ctx)->run(); //Started listener

    ctx.run();
}