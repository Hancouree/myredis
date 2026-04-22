#pragma once
#include <functional>
#include "ServerContext.h"
#include "../include/Utils.h"

class Handler {
public:
    virtual ~Handler() = default;
    virtual std::string execute(
        const std::vector<std::string>& args, 
        std::shared_ptr<ServerContext>& serverCtx
    ) = 0;
protected:
    template <typename Fn>
    std::string tryExecute(Fn&& func) {
        try { return func(); }
        catch (const std::exception& e) { return Utils::Resp::error(e.what()); }
    }
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

class IncrHandler : public Handler {
public:
    std::string execute(
        const std::vector<std::string>& args,
        std::shared_ptr<ServerContext>& serverCtx
    ) override;
};

class IncrByHandler : public Handler {
public:
    std::string execute(
        const std::vector<std::string>& args,
        std::shared_ptr<ServerContext>& serverCtx
    ) override;
};

class DecrHandler : public Handler {
public:
    std::string execute(
        const std::vector<std::string>& args,
        std::shared_ptr<ServerContext>& serverCtx
    ) override;
};

class DecrByHandler : public Handler {
public:
    std::string execute(
        const std::vector<std::string>& args,
        std::shared_ptr<ServerContext>& serverCtx
    ) override;
};

class StrlenHandler : public Handler {
public:
    std::string execute(
        const std::vector<std::string>& args,
        std::shared_ptr<ServerContext>& serverCtx
    ) override;
};

class AppendHandler : public Handler {
public:
    std::string execute(
        const std::vector<std::string>& args,
        std::shared_ptr<ServerContext>& serverCtx
    ) override;
};

class MGetHandler : public Handler {
public:
    std::string execute(
        const std::vector<std::string>& args,
        std::shared_ptr<ServerContext>& serverCtx
    ) override;
};

class ExistsHandler : public Handler {
public:
    std::string execute(
        const std::vector<std::string>& args,
        std::shared_ptr<ServerContext>& serverCtx
    ) override;
};

class TypeHandler : public Handler {
public:
    std::string execute(
        const std::vector<std::string>& args,
        std::shared_ptr<ServerContext>& serverCtx
    ) override;
};

class TtlHandler : public Handler {
public:
    std::string execute(
        const std::vector<std::string>& args,
        std::shared_ptr<ServerContext>& serverCtx
    ) override;
};

class PersistHandler : public Handler {
public:
    std::string execute(
        const std::vector<std::string>& args,
        std::shared_ptr<ServerContext>& serverCtx
    ) override;
};

class RenameHandler : public Handler {
public:
    std::string execute(
        const std::vector<std::string>& args,
        std::shared_ptr<ServerContext>& serverCtx
    ) override;
};

class KeysHandler : public Handler {
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

class LPopHandler : public Handler {
public:
    std::string execute(
        const std::vector<std::string>& args,
        std::shared_ptr<ServerContext>& serverCtx
    ) override;
};

class RPopHandler : public Handler {
public:
    std::string execute(
        const std::vector<std::string>& args,
        std::shared_ptr<ServerContext>& serverCtx
    ) override;
};

class LLenHandler : public Handler {
public:
    std::string execute(
        const std::vector<std::string>& args,
        std::shared_ptr<ServerContext>& serverCtx
    ) override;
};

class LIndexHandler : public Handler {
public:
    std::string execute(
        const std::vector<std::string>& args,
        std::shared_ptr<ServerContext>& serverCtx
    ) override;
};

class LRangeHandler : public Handler {
public:
    std::string execute(
        const std::vector<std::string>& args,
        std::shared_ptr<ServerContext>& serverCtx
    ) override;
};

class LInsertHandler : public Handler {
public:
    std::string execute(
        const std::vector<std::string>& args,
        std::shared_ptr<ServerContext>& serverCtx
    ) override;
};

class LSetHandler : public Handler {
public:
    std::string execute(
        const std::vector<std::string>& args,
        std::shared_ptr<ServerContext>& serverCtx
    ) override;
};

class LTrimHandler : public Handler {
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

class HKeysHandler : public Handler {
public:
    std::string execute(
        const std::vector<std::string>& args,
        std::shared_ptr<ServerContext>& serverCtx
    ) override;
};

class HValsHandler : public Handler {
public:
    std::string execute(
        const std::vector<std::string>& args,
        std::shared_ptr<ServerContext>& serverCtx
    ) override;
};

class HMGetHandler : public Handler {
public:
    std::string execute(
        const std::vector<std::string>& args,
        std::shared_ptr<ServerContext>& serverCtx
    ) override;
};