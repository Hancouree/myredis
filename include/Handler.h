#pragma once
#include <functional>
#include "ServerContext.h"

class Handler {
public:
    virtual ~Handler() = default;
    virtual void execute(
        const std::vector<std::string>& args, 
        std::shared_ptr<ServerContext>& serverCtx, 
        std::function<void(const std::string&)> callback
    ) = 0;
};

class PingHandler : public Handler {
public:
    void execute(
        const std::vector<std::string>& args,
        std::shared_ptr<ServerContext>& serverCtx,
        std::function<void(const std::string&)> callback
    ) override;
};

class SetHandler : public Handler {
public:
    void execute(
        const std::vector<std::string>& args,
        std::shared_ptr<ServerContext>& serverCtx,
        std::function<void(const std::string&)> callback
    ) override;
};

class GetHandler : public Handler {
public:
    void execute(
        const std::vector<std::string>& args,
        std::shared_ptr<ServerContext>& serverCtx,
        std::function<void(const std::string&)> callback
    ) override;
};

class ExpireHandler : public Handler {
public:
    void execute(
        const std::vector<std::string>& args,
        std::shared_ptr<ServerContext>& serverCtx,
        std::function<void(const std::string&)> callback
    ) override;
};

class DelHandler : public Handler {
public:
    void execute(
        const std::vector<std::string>& args,
        std::shared_ptr<ServerContext>& serverCtx,
        std::function<void(const std::string&)> callback
    ) override;
};

class InfoHandler : public Handler {
public:
    void execute(
        const std::vector<std::string>& args,
        std::shared_ptr<ServerContext>& serverCtx,
        std::function<void(const std::string&)> callback
    ) override;
};