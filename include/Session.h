#pragma once
#include <boost/asio.hpp>
#include <queue>
#include "ServerContext.h"
#include "Parser.h"

namespace asio = boost::asio;
using tcp = boost::asio::ip::tcp;

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(tcp::socket s, std::shared_ptr<ServerContext> serverCtx);
    ~Session();

    void run() { doRead(); }
private:
    void doRead();
    void doWrite(const std::string& msg);
    void doWriteNext();
    std::string handleCommand(std::vector<std::string>& args);

    tcp::socket m_socket;
    asio::streambuf m_buffer;
    Parser m_parser;
    std::shared_ptr<ServerContext> m_serverCtx;
    std::queue<std::string> m_writingQueue;
};

