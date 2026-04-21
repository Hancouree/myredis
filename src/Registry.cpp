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

    m_handlers["LPUSH"] = std::make_shared<LPushHandler>();
    m_handlers["RPUSH"] = std::make_shared<RPushHandler>();
    m_handlers["LPOP"] = std::make_shared<LPopHandler>();
    m_handlers["RPOP"] = std::make_shared<RPopHandler>();
    m_handlers["LLEN"] = std::make_shared<LLenHandler>();
    m_handlers["LINDEX"] = std::make_shared<LIndexHandler>();
    m_handlers["LRANGE"] = std::make_shared<LRangeHandler>();
    m_handlers["LINSERT"] = std::make_shared<LInsertHandler>();
    m_handlers["LSET"] = std::make_shared<LSetHandler>();
    m_handlers["LTRIM"] = std::make_shared<LTrimHandler>();

    m_handlers["HSET"] = std::make_shared<HSetHandler>();
    m_handlers["HGET"] = std::make_shared<HGetHandler>();
    m_handlers["HGETALL"] = std::make_shared<HGetAllHandler>();
    m_handlers["HDEL"] = std::make_shared<HDelHandler>();
    m_handlers["HEXISTS"] = std::make_shared<HExistsHandler>();
    m_handlers["HLEN"] = std::make_shared<HLenHandler>();
}

std::string Registry::handle(
    const std::string& cmd, 
    const std::vector<std::string>& args, 
    std::shared_ptr<ServerContext>& serverCtx
) {
    if (auto it = m_handlers.find(cmd); it != m_handlers.end()) {
        return it->second->execute(args, serverCtx);
    }

    return "-ERR unknown command\r\n";;
}
