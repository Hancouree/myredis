#include "../include/Registry.h"
#include "../include/Handler.h"

std::unordered_map<std::string, std::shared_ptr<Handler>> Registry::m_handlers;

void Registry::init()
{
    m_handlers["PING"] = std::make_shared<PingHandler>();
    m_handlers["SET"] = std::make_shared<SetHandler>();
    m_handlers["GET"] = std::make_shared<GetHandler>();
    m_handlers["EXPIRE"] = std::make_shared<ExpireHandler>();
    m_handlers["DEL"] = std::make_shared<DelHandler>();
    m_handlers["INFO"] = std::make_shared<InfoHandler>();
}

void Registry::handle(
    const std::string& cmd, 
    const std::vector<std::string>& args, 
    std::shared_ptr<ServerContext>& serverCtx, 
    std::function<void(const std::string&)> callback
)
{
    if (auto it = m_handlers.find(cmd); it != m_handlers.end()) {
        it->second->execute(args, serverCtx, callback);
    }
}
