#include "../include/Handler.h"
#include "../include/ServerContext.h"
#include <sstream>
#include <windows.h>

void PingHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx, std::function<void(const std::string&)> callback)
{
    callback("PONG\r\n");
}

void SetHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx, std::function<void(const std::string&)> callback)
{
    if (args.size() < 3) {
        callback("-ERR wrong number of arguments for SET\r\n");
        return;
    }

    serverCtx->m_repo->set(args[1], args[2]);
    callback("+OK\r\n");
}

void GetHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx, std::function<void(const std::string&)> callback)
{
    if (args.size() < 2) {
        callback("-ERR wrong number of arguments for GET\r\n");
        return;
    }

    std::string val = serverCtx->m_repo->get(args[1]);
    if (val.empty()) {
        callback("$-1\r\n");
    }
    else {
        callback("$" + std::to_string(val.size()) + "\r\n" + val + "\r\n");
    }
}

void ExpireHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx, std::function<void(const std::string&)> callback)
{
    if (args.size() < 3) {
        callback("-ERR wrong number of arguments for EXPIRE\r\n");
        return;
    }

    try
    {
        int seconds = std::stoi(args[2]);

        if (serverCtx->m_repo->expires(args[1], seconds)) {
            callback(":1\r\n");
        }
        else {
            callback(":0\r\n");
        }
    }
    catch (const std::exception&)
    {
        callback("-ERR value is not an integer or out of range\r\n");
    }
}

void DelHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx, std::function<void(const std::string&)> callback)
{
    if (args.size() < 2) {
        callback("-ERR wrong number of arguments for DEL\r\n");
        return;
    }

    if (serverCtx->m_repo->del(args[1])) {
        callback(":1\r\n");
    }
    else {
        callback(":0\r\n");
    }
}

void InfoHandler::execute(const std::vector<std::string>& args, std::shared_ptr<ServerContext>& serverCtx, std::function<void(const std::string&)> callback)
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
    callback("$" + std::to_string(result.size()) + "\r\n" + result + "\r\n");
}