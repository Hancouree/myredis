#include "../include/Handler.h"
#include "../include/ServerContext.h"
#include <windows.h>
#include <algorithm>

std::string PingHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    return Utils::Resp::pong();
}

std::string SetHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 3) {
        return Utils::Resp::error("wrong number of arguments for SET");
    }

    serverCtx->m_repo->set(args[1], args[2]);
    return Utils::Resp::ok();
}

std::string GetHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 2) {
        return Utils::Resp::error("wrong number of arguments for GET");
    }

    const RecordValue* val = serverCtx->m_repo->get(args[1]);
    if (!val) return Utils::Resp::nil();

    if (auto* s = std::get_if<String>(val)) {
        return Utils::Resp::bulk(*s);
    }
    else if (auto* l = std::get_if<List>(val)) {
        return Utils::Resp::list(*l);
    }
    else if (auto* h = std::get_if<Hash>(val)) {
        return Utils::Resp::hash(*h);
    }

    return Utils::Resp::error("internal error");
}

std::string ExpireHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 3) {
        return Utils::Resp::error("wrong number of arguments for EXPIRE");
    }

    return tryExecute([&]() {
        int seconds = std::stoi(args[2]);
        return serverCtx->m_repo->expires(args[1], seconds) ? Utils::Resp::integer(1) : Utils::Resp::integer(0);
    });
}

std::string DelHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 2) {
        return Utils::Resp::error("wrong number of arguments for DEL");
    }

    return Utils::Resp::integer(serverCtx->m_repo->del({ args.begin() + 1, args.end() }));
}

std::string InfoHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    auto now = std::chrono::steady_clock::now();

    std::stringstream ss;
    ss << "Server\r\n";
    ss << "server_version:1.0.0\r\n";
    ss << "uptime_in_seconds:" << std::chrono::duration_cast<std::chrono::seconds>(now - serverCtx->getStartTime()).count() << "\r\n";
    ss << "connected_clients:" << serverCtx->getConnections() << "\r\n";
    ss << "process_id:" << GetCurrentProcessId() << "\r\n\r\n";
    ss << "used_memory:" << serverCtx->m_repo->getMemoryUsed() << "\r\n";
    ss << "keys_count:" << serverCtx->m_repo->count() << "\r\n\r\n";
    ss << "total_connections_received:" << serverCtx->getAllConnections() << "\r\n";
    ss << "total_commands_processed:" << serverCtx->getAllProcessedCommands() << "\r\n";
    return Utils::Resp::bulk(ss.str());
}

std::string IncrHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 2) {
        return Utils::Resp::error("wrong number of arguments for INCR");
    }

    return tryExecute([&]() {
        return Utils::Resp::integer(serverCtx->m_repo->incrBy(args[1]));
    });
}

std::string IncrByHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 3) {
        return Utils::Resp::error("wrong number of arguments for INCRBY");
    }

    return tryExecute([&]() {
        return Utils::Resp::integer(serverCtx->m_repo->incrBy(args[1], std::stoi(args[2])));
    });
}

std::string DecrHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 2) {
        return Utils::Resp::error("wrong number of arguments for DECR");
    }

    return tryExecute([&]() {
        return Utils::Resp::integer(serverCtx->m_repo->incrBy(args[1], -1));
    });
}
std::string DecrByHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 3) return Utils::Resp::error("wrong number of arguments for DECRBY");
    return tryExecute([&]() {
        return Utils::Resp::integer(serverCtx->m_repo->incrBy(args[1], -std::stoi(args[2])));
    });
}

std::string StrlenHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 2) return Utils::Resp::error("wrong number of arguments for STRLEN");
    return tryExecute([&]() {
        return Utils::Resp::integer(serverCtx->m_repo->strlen(args[1]));
    });
}

std::string AppendHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 3) return Utils::Resp::error("wrong number of arguments for APPEND");
    return tryExecute([&]() {
        return Utils::Resp::integer(serverCtx->m_repo->append(args[1], args[2]));
    });
}

std::string MGetHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 2) return Utils::Resp::error("wrong number of arguments for MGET");
    auto results = serverCtx->m_repo->mget({ args.begin() + 1, args.end() });
    std::string out = "*" + std::to_string(results.size()) + "\r\n";
    for (const auto& r : results) out += Utils::Resp::nullableBulk(r);
    return out;
}

std::string ExistsHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 2) return Utils::Resp::error("wrong number of arguments for EXISTS");
    return Utils::Resp::integer(serverCtx->m_repo->exists({ args.begin() + 1, args.end() }));
}

std::string TypeHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 2) return Utils::Resp::error("wrong number of arguments for TYPE");
    const RecordValue* v = serverCtx->m_repo->get(args[1]);
    if (!v) return Utils::Resp::simple("none");
    return std::visit([](auto&& arg) -> std::string {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, String>) return Utils::Resp::simple("string");
        else if constexpr (std::is_same_v<T, List>) return Utils::Resp::simple("list");
        else if constexpr (std::is_same_v<T, Hash>) return Utils::Resp::simple("hash");
        else return Utils::Resp::error("internal error");
    }, *v);
}

std::string TtlHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 2) return Utils::Resp::error("wrong number of arguments for TTL");
    return Utils::Resp::integer(serverCtx->m_repo->ttl(args[1]));
}

std::string PersistHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 2) return Utils::Resp::error("wrong number of arguments for PERSIST");
    return Utils::Resp::integer(serverCtx->m_repo->persist(args[1]) ? 1 : 0);
}

std::string RenameHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 3) return Utils::Resp::error("wrong number of arguments for RENAME");
    return tryExecute([&]() {
        serverCtx->m_repo->rename(args[1], args[2]);
        return Utils::Resp::ok();
    });
}

std::string KeysHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 2) return Utils::Resp::error("wrong number of arguments for KEYS");
    return Utils::Resp::list(serverCtx->m_repo->keys(args[1]));
}

std::string LPushHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 3) return Utils::Resp::error("wrong number of arguments for LPUSH");
    return tryExecute([&]() {
        return Utils::Resp::integer(serverCtx->m_repo->lpush(args[1], args[2]));
    });
}

std::string RPushHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 3) return Utils::Resp::error("wrong number of arguments for RPUSH");
    return tryExecute([&]() {
        return Utils::Resp::integer(serverCtx->m_repo->rpush(args[1], args[2]));
    });
}

std::string LPopHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 2) return Utils::Resp::error("wrong number of arguments for LPOP");
    return tryExecute([&]() {
        return Utils::Resp::nullableBulk(serverCtx->m_repo->lpop(args[1]));
    });
}

std::string RPopHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 2) return Utils::Resp::error("wrong number of arguments for RPOP");
    return tryExecute([&]() {
        return Utils::Resp::nullableBulk(serverCtx->m_repo->rpop(args[1]));
    });
}

std::string LLenHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 2) return Utils::Resp::error("wrong number of arguments for LLEN");
    return tryExecute([&]() {
        return Utils::Resp::integer(serverCtx->m_repo->llen(args[1]));
    });
}

std::string LIndexHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 3) return Utils::Resp::error("wrong number of arguments for LINDEX");
    return tryExecute([&]() {
        return Utils::Resp::nullableBulk(serverCtx->m_repo->lindex(args[1], std::stoi(args[2])));
    });
}

std::string LRangeHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 4) return Utils::Resp::error("wrong number of arguments for LRANGE");
    return tryExecute([&]() {
        return Utils::Resp::list(serverCtx->m_repo->lrange(args[1], std::stoi(args[2]), std::stoi(args[3])));
    });
}

std::string LInsertHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 5) return Utils::Resp::error("wrong number of arguments for LINSERT");
    return tryExecute([&]() {
        std::string where = args[2];
        std::transform(where.begin(), where.end(), where.begin(), ::toupper);
        if (where != "BEFORE" && where != "AFTER")
            return Utils::Resp::error("syntax error");
        return Utils::Resp::integer(serverCtx->m_repo->linsert(args[1], where, args[3], args[4]));
    });
}

std::string LSetHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 4) return Utils::Resp::error("wrong number of arguments for LSET");
    return tryExecute([&]() {
        serverCtx->m_repo->lset(args[1], std::stoi(args[2]), args[3]);
        return Utils::Resp::ok();
    });
}

std::string LTrimHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 4) return Utils::Resp::error("wrong number of arguments for LTRIM");
    return tryExecute([&]() {
        serverCtx->m_repo->ltrim(args[1], std::stoi(args[2]), std::stoi(args[3]));
        return Utils::Resp::ok();
    });
}

std::string HSetHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 4) return Utils::Resp::error("wrong number of arguments for HSET");
    return tryExecute([&]() {
        return Utils::Resp::integer(serverCtx->m_repo->hset(args[1], args[2], args[3]));
    });
}

std::string HGetHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 3) return Utils::Resp::error("wrong number of arguments for HGET");
    return tryExecute([&]() {
        return Utils::Resp::nullableBulk(serverCtx->m_repo->hget(args[1], args[2]));
    });
}

std::string HGetAllHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 2) return Utils::Resp::error("wrong number of arguments for HGETALL");
    return tryExecute([&]() {
        const Hash* h = serverCtx->m_repo->hgetall(args[1]);
        return h ? Utils::Resp::hash(*h) : Utils::Resp::emptyArr();
    });
}

std::string HDelHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 3) return Utils::Resp::error("wrong number of arguments for HDEL");
    return tryExecute([&]() {
        return Utils::Resp::integer(serverCtx->m_repo->hdel(args[1], args[2]) ? 1 : 0);
    });
}

std::string HExistsHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 3) return Utils::Resp::error("wrong number of arguments for HEXISTS");
    return tryExecute([&]() {
        return Utils::Resp::integer(serverCtx->m_repo->hexists(args[1], args[2]) ? 1 : 0);
    });
}

std::string HLenHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 2) return Utils::Resp::error("wrong number of arguments for HLEN");
    return tryExecute([&]() {
        return Utils::Resp::integer(serverCtx->m_repo->hlen(args[1]));
    });
}

std::string HKeysHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 2) return Utils::Resp::error("wrong number of arguments for HKEYS");
    return tryExecute([&]() {
        return Utils::Resp::list(serverCtx->m_repo->hkeys(args[1]));
    });
}

std::string HValsHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 2) return Utils::Resp::error("wrong number of arguments for HVALS");
    return tryExecute([&]() {
        return Utils::Resp::list(serverCtx->m_repo->hvals(args[1]));
    });
}

std::string HMGetHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 3) return Utils::Resp::error("wrong number of arguments for HMGET");
    return tryExecute([&]() {
        auto results = serverCtx->m_repo->hmget(args[1], { args.begin() + 2, args.end() });
        std::string out = "*" + std::to_string(results.size()) + "\r\n";
        for (const auto& r : results) out += Utils::Resp::nullableBulk(r);
        return out;
    });
}