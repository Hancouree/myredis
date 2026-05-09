#include "../include/Listener.h"
#include "../include/Session.h"
#include "../include/Config.h"
#include "../include/ServerContext.h"
#include <iostream>

Listener::Listener(asio::io_context& ctx, std::shared_ptr<ServerContext> serverCtx)
    : m_acceptor(ctx)
    , m_serverCtx(serverCtx)
{
    m_maxClients = m_serverCtx->config().getInt("maxclients", 100);

    std::string bind = m_serverCtx->config().getStr("bind", "127.0.0.1");
    uint16_t port = m_serverCtx->config().getInt("port", 5050);
    int backlog = m_serverCtx->config().getInt("tcp-backlog", 511);

    tcp::endpoint endpoint(asio::ip::make_address(bind), port);

    m_acceptor.open(endpoint.protocol());
    m_acceptor.set_option(tcp::acceptor::reuse_address(true));
    m_acceptor.bind(endpoint);
    m_acceptor.listen(backlog);
}

void Listener::doAccept()
{
    m_acceptor.async_accept(
        [self = shared_from_this()](boost::system::error_code ec, tcp::socket s) {
            if (!ec) {
                if (self->m_serverCtx->getConnections() > self->m_maxClients) {
                    std::cerr << "[Warning] Max clients reached (" << self->m_maxClients << "). Connection rejected." << std::endl;
                }
                else {
                    std::make_shared<Session>(std::move(s), self->m_serverCtx)->run();
                }
            }

            self->doAccept();
        });
}