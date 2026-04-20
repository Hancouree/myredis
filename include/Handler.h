#pragma once
#include <functional>
#include "ServerContext.h"

class Handler {
public:
    virtual ~Handler() = default;
    virtual std::string execute(
        const std::vector<std::string>& args, 
        std::shared_ptr<ServerContext>& serverCtx
    ) = 0;
};

class PingHandler : public Handler {
public:
    std::string execute(
        const std::vector<std::string>& args,
        std::shared_ptr<ServerContext>& serverCtx
    ) override;
};

class SetHandler : public Handler {
public:
    std::string execute(
        const std::vector<std::string>& args,
        std::shared_ptr<ServerContext>& serverCtx
    ) override;
};

class GetHandler : public Handler {
public:
    std::string execute(
        const std::vector<std::string>& args,
        std::shared_ptr<ServerContext>& serverCtx
    ) override;
};

class ExpireHandler : public Handler {
public:
    std::string execute(
        const std::vector<std::string>& args,
        std::shared_ptr<ServerContext>& serverCtx
    ) override;
};

class DelHandler : public Handler {
public:
    std::string execute(
        const std::vector<std::string>& args,
        std::shared_ptr<ServerContext>& serverCtx
    ) override;
};

class InfoHandler : public Handler {
public:
    std::string execute(
        const std::vector<std::string>& args,
        std::shared_ptr<ServerContext>& serverCtx
    ) override;
};

class LPushHandler : public Handler {
public:
    std::string execute(
        const std::vector<std::string>& args,
        std::shared_ptr<ServerContext>& serverCtx
    ) override;
};

class RPushHandler : public Handler {
public:
    std::string execute(
        const std::vector<std::string>& args,
        std::shared_ptr<ServerContext>& serverCtx
    ) override;
};

class HSetHandler : public Handler {
public:
    std::string execute(
        const std::vector<std::string>& args,
        std::shared_ptr<ServerContext>& serverCtx
    ) override;
};

class HGetHandler : public Handler {
public:
    std::string execute(
        const std::vector<std::string>& args,
        std::shared_ptr<ServerContext>& serverCtx
    ) override;
};

class HGetAllHandler : public Handler {
public:
    std::string execute(
        const std::vector<std::string>& args,
        std::shared_ptr<ServerContext>& serverCtx
    ) override;
};

class HDelHandler : public Handler {
public:
    std::string execute(
        const std::vector<std::string>& args,
        std::shared_ptr<ServerContext>& serverCtx
    ) override;
};

class HExistsHandler : public Handler {
public:
    std::string execute(
        const std::vector<std::string>& args,
        std::shared_ptr<ServerContext>& serverCtx
    ) override;
};

class HLenHandler : public Handler {
public:
    std::string execute(
        const std::vector<std::string>& args,
        std::shared_ptr<ServerContext>& serverCtx
    ) override;
};