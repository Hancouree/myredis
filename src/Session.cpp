#include "../include/Session.h"
#include "../include/Registry.h"
#include "../include/Config.h"
#include <iostream>
#include <numeric>

Session::Session(tcp::socket s, std::shared_ptr<ServerContext> serverCtx)
    : m_socket(std::move(s))
    , m_serverCtx(serverCtx)
    , m_timer(m_socket.get_executor())
{
    m_serverCtx->incrementConnections();
}

Session::~Session()
{
    for (const auto& pattern : m_subscribedPatterns) {
        m_serverCtx->pubsubRepo().punsubscribe(pattern, this);
    }
    for (const auto& channel : m_subscribedChannels) {
        m_serverCtx->pubsubRepo().unsubscribe(channel, this);
    }

    m_serverCtx->decrementConnections();
}

void Session::addChannel(const std::string& channel)
{
    m_subscribedChannels.insert(channel);
}

void Session::addPattern(const std::string& pattern)
{
    m_subscribedPatterns.insert(pattern);
}

void Session::removeChannel(const std::string& channel)
{
    m_subscribedChannels.erase(channel);
}

void Session::removePattern(const std::string& pattern)
{
    m_subscribedPatterns.erase(pattern);
}

void Session::doRead()
{
    resetTimeout();

    m_socket.async_read_some(m_buffer.prepare(1024),
        [self = shared_from_this()](boost::system::error_code ec, size_t received) {
            if (!ec) {
                self->m_buffer.commit(received);

                std::vector<std::string> results;
                std::vector<std::string> args;
                while (self->m_parser.parse(self->m_buffer, args)) {
                    std::string res = self->handleCommand(args);
                    if (!res.empty()) {
                        results.push_back(res);
                        self->m_serverCtx->incrementProcessedCommands();
                    }

                    args.clear();
                }

                if (!results.empty()) {
                    size_t totalSize = 0;
                    for (const auto& s : results) totalSize += s.size();
                    std::string answer;
                    answer.reserve(totalSize);
                    for (const auto& s : results) answer += s;

                    self->doWrite(answer);
                }

                self->doRead();
            }
            else {
                if (ec != boost::asio::error::eof && ec != boost::asio::error::connection_reset) {
                    std::cout << "Read error: " << ec.message() << "\n";
                }
            }
        });
}

void Session::doWrite(const std::string& msg)
{
    resetTimeout();
    bool in_progress = !m_writingQueue.empty();
    m_writingQueue.push(msg);

    if (!in_progress) {
        doWriteNext();
    }
}

void Session::doWriteNext()
{
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

std::string Session::handleCommand(std::vector<std::string>& args)
{
    if (args.empty()) return "";

    std::string cmd = args[0];
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);

    if (isSubscribed()) {
        if (cmd != "SUBSCRIBE" && cmd != "UNSUBSCRIBE" &&
            cmd != "PSUBSCRIBE" && cmd != "PUNSUBSCRIBE" &&
            cmd != "PING" && cmd != "QUIT")
        {
            return "-ERR only (P)SUBSCRIBE / (P)UNSUBSCRIBE / PING / QUIT allowed in this context\r\n";
        }
    }

    return Registry::handle(cmd, args, m_serverCtx, this);
}

void Session::resetTimeout()
{
    int timeout;
    if (isSubscribed()) {
        timeout = m_serverCtx->config().getInt("subscribed_timeout", 3600);
    }
    else {
        timeout = m_serverCtx->config().getInt("timeout", 300);
    }

    m_timer.expires_after(std::chrono::seconds(timeout));

    auto self(shared_from_this());
    m_timer.async_wait([this, self](boost::system::error_code ec) {
        if (!ec) {
            std::cout << "Session timeout. Closing connection.\n";
            boost::system::error_code close_ec;
            m_socket.close(close_ec);
        }
    });
}
