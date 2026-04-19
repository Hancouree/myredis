#pragma once
#include <boost/asio.hpp>
#include "ServerContext.h"

namespace asio = boost::asio;

class Cleaner
{
public:
	Cleaner(asio::io_context& ctx, std::shared_ptr<ServerContext> serverCtx);
private:
	void doWait();
	void performCleanup();

	asio::steady_timer m_timer;
	std::shared_ptr<ServerContext> m_serverCtx;
};

