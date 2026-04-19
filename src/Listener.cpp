#include "../include/Listener.h"
#include "../include/Session.h"

Listener::Listener(asio::io_context& ctx, std::shared_ptr<ServerContext> serverCtx)
    : m_acceptor(ctx, { boost::asio::ip::make_address("127.0.0.1"), 5050 })
    , m_serverCtx(serverCtx)
{
}

void Listener::doAccept()
{
    m_acceptor.async_accept(
        [self = shared_from_this()](boost::system::error_code ec, tcp::socket s) {
            if (!ec) {
                std::make_shared<Session>(std::move(s), self->m_serverCtx)->run();
            }

            self->doAccept();
        });
}