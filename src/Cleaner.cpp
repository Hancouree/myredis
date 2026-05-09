#include "../include/Cleaner.h"
#include "../include/Config.h"
#include <iostream>

Cleaner::Cleaner(asio::io_context& ctx, std::shared_ptr<ServerContext> serverCtx)
	: m_timer(ctx)
	, m_serverCtx(serverCtx)
{
	int hz = m_serverCtx->config().getInt("hz", 10);
	if (hz < 0) hz = 1;

	m_timeout = 1000 / hz;

	m_timer.expires_after(std::chrono::milliseconds(m_timeout));
	doWait();
}

void Cleaner::doWait()
{
	m_timer.async_wait([this](boost::system::error_code ec) {
		if (!ec) {
			performCleanup();
			m_timer.expires_after(std::chrono::milliseconds(m_timeout));
			doWait();
		}
		else {
			std::cout << "Cleaner died: " << ec.message() << "\n";
		}
	});
}

void Cleaner::performCleanup()
{
	m_serverCtx->repo().performCleanup();
}