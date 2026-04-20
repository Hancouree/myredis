#include "../include/Handler.h"
#include "../include/ServerContext.h"
#include <sstream>
#include <windows.h>

std::string PingHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    return "+PONG\r\n";
}

std::string SetHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 3) {
        return "-ERR wrong number of arguments for SET\r\n";
    }

    serverCtx->m_repo->set(args[1], args[2]);
    return "+OK\r\n";
}

std::string GetHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 2) {
        return "-ERR wrong number of arguments for GET\r\n";
    }

    const RecordValue* val = serverCtx->m_repo->get(args[1]);
    if (!val) return "$-1\r\n";

    if (auto* s = std::get_if<String>(val)) {
        return "$" + std::to_string(s->size()) + "\r\n" + *s + "\r\n";
    }
    else if (auto* l = std::get_if<List>(val)) {
        std::stringstream ss;
        ss << "*" << l->size() << "\r\n";
        for (const auto& e : *l) {
            ss << "$" << e.size() << "\r\n" << e << "\r\n";
        }

        return ss.str();
    }
    else if (auto* h = std::get_if<Hash>(val)) {
        std::stringstream ss;
        ss << "*" << h->size() * 2 << "\r\n";
        for (auto it = h->begin(); it != h->end(); ++it) {
            ss << "$" << it->first.size() << "\r\n" << it->first << "\r\n";
            ss << "$" << it->second.size() << "\r\n" << it->second << "\r\n";
        }
        return ss.str();
    }

    return "-ERR internal error\r\n";
}

std::string ExpireHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 3) {
        return "-ERR wrong number of arguments for EXPIRE\r\n";
    }

    try
    {
        int seconds = std::stoi(args[2]);

        if (serverCtx->m_repo->expires(args[1], seconds)) {
            return ":1\r\n";
        }
        else {
            return ":0\r\n";
        }
    }
    catch (const std::exception&)
    {
        return "-ERR value is not an integer or out of range\r\n";
    }
}

std::string DelHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 2) {
        return "-ERR wrong number of arguments for DEL\r\n";
    }

    if (serverCtx->m_repo->del(args[1])) {
        return ":1\r\n";
    }
    else {
        return ":0\r\n";
    }
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

    std::string result = ss.str();
    return "$" + std::to_string(result.size()) + "\r\n" + result + "\r\n";
}

std::string LPushHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 3) {
        return "-ERR wrong number of arguments for LPUSH\r\n";
    }

    try
    {
        int size = serverCtx->m_repo->lpush(args[1], args[2]);
        return ":" + std::to_string(size) + "\r\n";
    }
    catch (const std::runtime_error& e)
    {
        return std::string("-ERR ") + e.what() + "\r\n";
    }
}

std::string RPushHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 3) {
        return "-ERR wrong number of arguments for RPUSH\r\n";
    }

    try
    {
        int size = serverCtx->m_repo->rpush(args[1], args[2]);
        return ":" + std::to_string(size) + "\r\n";
    }
    catch (const std::runtime_error& e)
    {
        return std::string("-ERR ") + e.what() + "\r\n";
    }
}

std::string HSetHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 4) {
        return "-ERR wrong number of arguments for HSET\r\n";
    }

    try
    {
        int r = serverCtx->m_repo->hset(args[1], args[2], args[3]);
        return ":" + std::to_string(r) + "\r\n";
    }
    catch (const std::runtime_error& e)
    {
        return std::string("-ERR ") + e.what() + "\r\n";
    }
}

std::string HGetHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 3) {
        return "-ERR wrong number of arguments for HGET\r\n";
    }

    try
    {
        std::optional<String> val = serverCtx->m_repo->hget(args[1], args[2]);
        if (!val.has_value()) {
            return "$-1\r\n";
        }

        return "$" + std::to_string(val->size()) + "\r\n" + val.value() + "\r\n";
    }
    catch (const std::runtime_error& e)
    {
        return std::string("-ERR ") + e.what() + "\r\n";
    }
}

std::string HGetAllHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 2) {
        return "-ERR wrong number of arguments for HGETALL\r\n";
    }

    try
    {
        const Hash* h = serverCtx->m_repo->hgetall(args[1]);
        if (!h) return "*0\r\n";

        std::stringstream ss;
        ss << "*" << h->size() * 2 << "\r\n";
        for (auto it = h->begin(); it != h->end(); ++it) {
            ss << "$" << it->first.size() << "\r\n" << it->first << "\r\n";
            ss << "$" << it->second.size() << "\r\n" << it->second << "\r\n";
        }
        return ss.str();
    }
    catch (const std::runtime_error& e)
    {
        return std::string("-ERR ") + e.what() + "\r\n";
    }
}

std::string HDelHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 3) {
        return "-ERR wrong number of arguments for HDEL\r\n";
    }

    try
    {
        bool deleted = serverCtx->m_repo->hdel(args[1], args[2]);
        return deleted ? ":1\r\n" : ":0\r\n";
    }
    catch (const std::runtime_error& e)
    {
        return std::string("-ERR ") + e.what() + "\r\n";
    }
}

std::string HExistsHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 3) {
        return "-ERR wrong number of arguments for HEXISTS\r\n";
    }

    try
    {
        return serverCtx->m_repo->hexists(args[1], args[2]) ? ":1\r\n" : ":0\r\n";
    }
    catch (const std::runtime_error& e)
    {
        return std::string("-ERR ") + e.what() + "\r\n";
    }
}

std::string HLenHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    if (args.size() < 2) {
        return "-ERR wrong number of arguments for HLEN\r\n";
    }

    try
    {
        return ":" + std::to_string(serverCtx->m_repo->hlen(args[1])) + "\r\n";
    }
    catch (const std::runtime_error& e)
    {
        return std::string("-ERR ") + e.what() + "\r\n";
    }
}