#include "../include/Session.h"
#include "../include/Registry.h"
#include <iostream>

Session::Session(tcp::socket s, std::shared_ptr<ServerContext> serverCtx)
    : m_socket(std::move(s))
    , m_serverCtx(serverCtx)
{
    m_serverCtx->incrementConnections();
}

Session::~Session()
{
    m_serverCtx->decrementConnections();
}

void Session::doRead()
{
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

void Session::doWrite(const std::string& msg)
{
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

void Session::handleCommand(std::vector<std::string>& args)
{
    if (args.empty()) return;

    Registry::handle(
        args[0],
        args,
        m_serverCtx,
        [this](const std::string& answer) {
            m_serverCtx->incrementProcessedCommands();
            doWrite(answer);
        }
    );
}