#include "../include/Cleaner.h"
#include <iostream>

Cleaner::Cleaner(asio::io_context& ctx, std::shared_ptr<ServerContext> serverCtx)
	: m_timer(ctx)
	, m_serverCtx(serverCtx)
{
	m_timer.expires_after(std::chrono::seconds(1));
	doWait();
}

void Cleaner::doWait()
{
	m_timer.async_wait([this](boost::system::error_code ec) {
		if (!ec) {
			performCleanup();
			m_timer.expires_after(std::chrono::seconds(1));
			doWait();
		}
		else {
			std::cout << "Cleaner died: " << ec.message() << "\n";
		}
	});
}

void Cleaner::performCleanup()
{
	m_serverCtx->m_repo->performCleanup();
}