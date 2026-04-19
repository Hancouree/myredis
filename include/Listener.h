#pragma once
#include <boost/asio.hpp>
#include "ServerContext.h"

namespace asio = boost::asio;
using tcp = boost::asio::ip::tcp;

class Listener : public std::enable_shared_from_this<Listener> {
public:
    Listener(asio::io_context& ctx, std::shared_ptr<ServerContext> serverCtx);

    void run() { doAccept(); }
private:
    void doAccept();

    tcp::acceptor m_acceptor;
    std::shared_ptr<ServerContext> m_serverCtx;
};