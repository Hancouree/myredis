#include "include/ServerContext.h"
#include "include/Registry.h"
#include "include/Listener.h"
#include "include/Cleaner.h"
#include "include/CommandDocs.h"
#include "include/Config.h"

int main()
{
    auto cfg = std::make_shared<Config>("myredis.conf");
    auto serverCtx = std::make_shared<ServerContext>(cfg);
    
    Registry::init();
    CommandDocs::init();

    asio::io_context ctx;

    auto cleaner = std::make_shared<Cleaner>(ctx, serverCtx);

    std::make_shared<Listener>(ctx, serverCtx)->run();

    ctx.run();
}