#pragma once
#include <unordered_map>
#include <string>
#include "Handler.h"

class Registry {
public:
    Registry() = delete;

    static void init();

    static std::optional<std::string> handle(
        const std::string& cmd,
        const std::vector<std::string>& args,
        std::shared_ptr<ServerContext>& serverCtx
    );
private:
    static std::unordered_map<std::string, std::shared_ptr<Handler>> m_handlers;
};