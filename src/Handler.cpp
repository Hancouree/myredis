#include "../include/Handler.h"
#include "../include/ServerContext.h"
#include <sstream>
#include <windows.h>

std::string PingHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx)
{
    return "PONG\r\n";
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

    std::string val = serverCtx->m_repo->get(args[1]);
    if (val.empty()) {
        return "$-1\r\n";
    }
    else {
        return "$" + std::to_string(val.size()) + "\r\n" + val + "\r\n";
    }
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