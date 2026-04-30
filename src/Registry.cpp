#include "../include/Registry.h"
#include "../include/Handler.h"
#include "../include/SessionHandler.h"

std::unordered_map<std::string, std::shared_ptr<Handler>> Registry::m_handlers;
std::unordered_map<std::string, std::shared_ptr<SessionHandler>> Registry::m_sessionHandlers;

void Registry::init()
{
    m_handlers["PING"] = std::make_shared<PingHandler>();
    m_handlers["SET"] = std::make_shared<SetHandler>();
    m_handlers["GET"] = std::make_shared<GetHandler>();
    m_handlers["EXPIRE"] = std::make_shared<ExpireHandler>();
    m_handlers["DEL"] = std::make_shared<DelHandler>();
    m_handlers["INFO"] = std::make_shared<InfoHandler>();
    m_handlers["INCR"] = std::make_shared<IncrHandler>();
    m_handlers["INCRBY"] = std::make_shared<IncrByHandler>();
    m_handlers["DECR"] = std::make_shared<DecrHandler>();
    m_handlers["DECRBY"] = std::make_shared<DecrByHandler>();
    m_handlers["APPEND"] = std::make_shared<AppendHandler>();
    m_handlers["STRLEN"] = std::make_shared<StrlenHandler>();
    m_handlers["MGET"] = std::make_shared<MGetHandler>();
    m_handlers["EXISTS"] = std::make_shared<ExistsHandler>();
    m_handlers["TYPE"] = std::make_shared<TypeHandler>();
    m_handlers["TTL"] = std::make_shared<TtlHandler>();
    m_handlers["PERSIST"] = std::make_shared<PersistHandler>();
    m_handlers["RENAME"] = std::make_shared<RenameHandler>();
    m_handlers["KEYS"] = std::make_shared<KeysHandler>();

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
    m_handlers["HKEYS"] = std::make_shared<HKeysHandler>();
    m_handlers["HVALS"] = std::make_shared<HValsHandler>();
    m_handlers["HMGET"] = std::make_shared<HMGetHandler>();

    //SESSION HANDLERS
    m_sessionHandlers["SUBSCRIBE"] = std::make_shared<SubscribeHandler>();
    m_sessionHandlers["UNSUBSCRIBE"] = std::make_shared<UnsubscribeHandler>();
    m_sessionHandlers["PUBLISH"] = std::make_shared<PublishHandler>();
    m_sessionHandlers["PSUBSCRIBE"] = std::make_shared<PSubscribeHandler>();
}

std::string Registry::handle(
    const std::string& cmd, 
    const std::vector<std::string>& args, 
    std::shared_ptr<ServerContext>& serverCtx,
    Session* session
) {
    if (auto it = m_sessionHandlers.find(cmd); it != m_sessionHandlers.end()) {
        return it->second->execute(args, serverCtx, session);
    }

    if (auto it = m_handlers.find(cmd); it != m_handlers.end()) {
        return it->second->execute(args, serverCtx);
    }

    return "-ERR unknown command\r\n";
}
