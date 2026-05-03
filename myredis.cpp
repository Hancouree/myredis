#include "include/ServerContext.h"
#include "include/Registry.h"
#include "include/Listener.h"
#include "include/Cleaner.h"
#include "include/CommandDocs.h"

int main()
{
    auto serverCtx = std::make_shared<ServerContext>();
    Registry::init();
    CommandDocs::init();

    asio::io_context ctx;

    auto cleaner = std::make_shared<Cleaner>(ctx, serverCtx);

    std::make_shared<Listener>(ctx, serverCtx)->run();

    ctx.run();
}