#include "../include/Handler.h"
#include "../include/ServerContext.h"
#include "../include/Utils.h"
#include <windows.h>
#include <algorithm>

std::string PingHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    return Utils::pong();
}

std::string SetHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 3) {
        return Utils::error("wrong number of arguments for SET");
    }

    serverCtx->m_repo->set(args[1], args[2]);
    return Utils::ok();
}

std::string GetHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 2) {
        return Utils::error("wrong number of arguments for GET");
    }

    const RecordValue* val = serverCtx->m_repo->get(args[1]);
    if (!val) return Utils::nil();

    if (auto* s = std::get_if<String>(val)) {
        return Utils::bulk(*s);
    }
    else if (auto* l = std::get_if<List>(val)) {
        return Utils::list(*l);
    }
    else if (auto* h = std::get_if<Hash>(val)) {
        return Utils::hash(*h);
    }

    return Utils::error("internal error");
}

std::string ExpireHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 3) {
        return Utils::error("wrong number of arguments for EXPIRE");
    }

    try
    {
        int seconds = std::stoi(args[2]);
        return serverCtx->m_repo->expires(args[1], seconds) ? Utils::integer(1) : Utils::integer(0);
    }
    catch (const std::exception& e)
    {
        return Utils::error(e.what());
    }
}

std::string DelHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 2) {
        return Utils::error("wrong number of arguments for DEL");
    }

    return serverCtx->m_repo->del(args[1]) ? Utils::integer(1) : Utils::integer(0);
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
    return Utils::bulk(ss.str());
}

std::string LPushHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 3) {
        return Utils::error("wrong number of arguments for LPUSH");
    }

    try
    {
        int size = serverCtx->m_repo->lpush(args[1], args[2]);
        return Utils::integer(size);
    }
    catch (const std::exception& e)
    {
        return Utils::error(e.what());
    }
}

std::string RPushHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 3) {
        return Utils::error("wrong number of arguments for RPUSH");
    }

    try
    {
        int size = serverCtx->m_repo->rpush(args[1], args[2]);
        return Utils::integer(size);
    }
    catch (const std::exception& e)
    {
        return Utils::error(e.what());
    }
}

std::string LPopHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx) {
    if (args.size() < 2) {
        return Utils::error("wrong number of arguments for LPOP");
    }

    try
    {
        std::optional<String> val = serverCtx->m_repo->lpop(args[1]);
        if (!val.has_value()) return Utils::nil();
        return Utils::bulk(val.value());
    }
    catch (const std::exception& e)
    {
        return Utils::error(e.what());
    }
}

std::string RPopHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 2) {
        return Utils::error("wrong number of arguments for RPOP");
    }

    try
    {
        std::optional<String> val = serverCtx->m_repo->rpop(args[1]);
        if (!val.has_value()) return Utils::nil();
        return Utils::bulk(val.value());
    }
    catch (const std::exception& e)
    {
        return Utils::error(e.what());
    }
}

std::string LLenHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 2) {
        return Utils::error("wrong number of arguments for LLEN");
    }

    try
    {
        return Utils::integer(serverCtx->m_repo->llen(args[1]));
    }
    catch (const std::exception& e)
    {
        return Utils::error(e.what());
    }
}

std::string LIndexHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 3) {
        return Utils::error("wrong number of arguments for LINDEX");
    }

    try
    {
        std::optional<std::string> val = serverCtx->m_repo->lindex(args[1], std::stoi(args[2]));
        if (!val.has_value()) return Utils::nil();
        return Utils::bulk(val.value());
    }
    catch (const std::exception& e)
    {
        return Utils::error(e.what());
    }
}

std::string LRangeHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 4) {
        return Utils::error("wrong number of arguments for LRANGE");
    }

    try
    {
        const List& l = serverCtx->m_repo->lrange(args[1], std::stoi(args[2]), std::stoi(args[3]));
        return Utils::list(l);
    }
    catch (const std::exception& e)
    {
        return Utils::error(e.what());
    }
}

std::string LInsertHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 5) {
        return Utils::error("wrong number of arguments for LINSERT");
    }

    try
    {
        std::string where = args[2];
        std::transform(where.begin(), where.end(), where.begin(), ::toupper);
        if (where != "BEFORE" && where != "AFTER") {
            return Utils::error("syntax error");
        }

        int size = serverCtx->m_repo->linsert(args[1], where, args[3], args[4]);
        return Utils::integer(size);
    }
    catch (const std::exception& e)
    {
        return Utils::error(e.what());
    }
}

std::string LSetHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 4) {
        return Utils::error("wrong number of arguments for LSET");
    }

    try
    {
        serverCtx->m_repo->lset(args[1], std::stoi(args[2]), args[3]);
        return Utils::ok();
    }
    catch (const std::exception& e)
    {
        return Utils::error(e.what());
    }
}

std::string LTrimHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 4) {
        return Utils::error("wrong number of arguments for LTRIM");
    }

    try
    {
        serverCtx->m_repo->ltrim(args[1], std::stoi(args[2]), std::stoi(args[3]));
        return Utils::ok();
    }
    catch (const std::exception& e)
    {
        return Utils::error(e.what());
    }
}

std::string HSetHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 4) {
        return Utils::error("wrong number of arguments for HSET");
    }

    try
    {
        int size = serverCtx->m_repo->hset(args[1], args[2], args[3]);
        return Utils::integer(size);
    }
    catch (const std::exception& e)
    {
        return Utils::error(e.what());
    }
}

std::string HGetHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 3) {
        return Utils::error("wrong number of arguments for HGET");
    }

    try
    {
        std::optional<String> r = serverCtx->m_repo->hget(args[1], args[2]);
        return Utils::nullableBulk(r);
    }
    catch (const std::exception& e)
    {
        return Utils::error(e.what());
    }
}

std::string HGetAllHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 2) {
        return Utils::error("wrong number of arguments for HGETALL");
    }

    try
    {
        const Hash* h = serverCtx->m_repo->hgetall(args[1]);
        if (!h) return Utils::emptyArr();
        return Utils::hash(*h);
    }
    catch (const std::exception& e)
    {
        return Utils::error(e.what());
    }
}

std::string HDelHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 3) {
        return Utils::error("wrong number of arguments for HDEL");
    }

    try
    {
        return serverCtx->m_repo->hdel(args[1], args[2]) ? Utils::integer(1) : Utils::integer(0);
    }
    catch (const std::exception& e)
    {
        return Utils::error(e.what());
    }
}

std::string HExistsHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 3) {
        return Utils::error("wrong number of arguments for HEXISTS");
    }

    try
    {
        return serverCtx->m_repo->hexists(args[1], args[2]) ? Utils::integer(1) : Utils::integer(0);
    }
    catch (const std::exception& e)
    {
        return Utils::error(e.what());
    }
}

std::string HLenHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 2) {
        return Utils::error("wrong number of arguments for HLEN");
    }

    try
    {
        return Utils::integer(serverCtx->m_repo->hlen(args[1]));
    }
    catch (const std::exception& e)
    {
        return Utils::error(e.what());
    }
}

std::string HKeysHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 2) {
        return Utils::error("wrong number of arguments for HKEYS");
    }

    try
    {
        return Utils::list(serverCtx->m_repo->hkeys(args[1]));
    }
    catch (const std::exception& e)
    {
        return Utils::error(e.what());
    }
}

std::string HValsHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 2) {
        return Utils::error("wrong number of arguments for HVALS");
    }

    try
    {
        return Utils::list(serverCtx->m_repo->hvals(args[1]));
    }
    catch (const std::exception& e)
    {
        return Utils::error(e.what());
    }
}

std::string HMGetHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 3) {
        return Utils::error("wrong number of arguments for HMGET");
    }

    try
    {
        std::vector<std::optional<std::string>> results = serverCtx->m_repo->hmget(args[1], { args.begin() + 2, args.end() });
        std::string out = "*" + std::to_string(results.size()) + "\r\n";
        for (const auto& r : results) out += Utils::nullableBulk(r);
        return out;
    }
    catch (const std::exception& e)
    {
        return Utils::error(e.what());
    }
}