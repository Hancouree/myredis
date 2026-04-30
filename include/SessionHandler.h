#pragma once
#include <vector>
#include "ServerContext.h"

class Session;

class SessionHandler
{
public:
	virtual ~SessionHandler() = default;
	virtual std::string execute(
		const std::vector<std::string>& args,
		std::shared_ptr<ServerContext>& serverCtx,
		Session* session
	) = 0;
};

class SubscribeHandler : public SessionHandler {
public:
	std::string execute(
		const std::vector<std::string>& args,
		std::shared_ptr<ServerContext>& serverCtx,
		Session* session
	);
};

class PublishHandler : public SessionHandler {
public:
	std::string execute(
		const std::vector<std::string>& args,
		std::shared_ptr<ServerContext>& serverCtx,
		Session* session
	);
};

class UnsubscribeHandler : public SessionHandler {
public:
	std::string execute(
		const std::vector<std::string>& args,
		std::shared_ptr<ServerContext>& serverCtx,
		Session* session
	);
};

class PSubscribeHandler : public SessionHandler {
public:
	std::string execute(
		const std::vector<std::string>& args,
		std::shared_ptr<ServerContext>& serverCtx,
		Session* session
	);
};