#include "include/ServerContext.h"
#include "include/Registry.h"
#include "include/Listener.h"

int main()
{
    auto serverCtx = std::make_shared<ServerContext>();
    Registry::init();

    asio::io_context ctx;

    std::make_shared<Listener>(ctx, serverCtx)->run(); //Started listener

    ctx.run();
}