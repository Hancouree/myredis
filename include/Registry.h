#pragma once
#include <unordered_map>
#include <string>
#include <memory>

class Session;
class ServerContext;
class Handler;
class SessionHandler;

class Registry {
public:
    Registry() = delete;
    static void init();
    static std::string handle(
        const std::string& cmd,
        const std::vector<std::string>& args,
        std::shared_ptr<ServerContext>& serverCtx,
        Session* session
    );
private:
    static std::unordered_map<std::string, std::shared_ptr<Handler>> m_handlers;
    static std::unordered_map<std::string, std::shared_ptr<SessionHandler>> m_sessionHandlers;
};