#include "../include/ServerContext.h"
#include "../include/Session.h"
#include "../include/Utils.h"

ServerContext::ServerContext()
    : m_activeConnections(0)
    , m_allConnections(0)
    , m_processedCommands(0)
    , m_repo(std::make_shared<Repository>())
    , m_pubSubRepo(std::make_shared<PubSubRepository>())
    , m_startTime(std::chrono::steady_clock::now())
{
}

void ServerContext::incrementConnections()
{
    ++m_activeConnections; ++m_allConnections;
}

void ServerContext::decrementConnections()
{
    --m_activeConnections;
}

void ServerContext::incrementProcessedCommands()
{
    ++m_processedCommands;
}

int ServerContext::getConnections() const
{
    return m_activeConnections;
}

int ServerContext::getAllConnections() const
{
    return m_allConnections;
}

int ServerContext::getAllProcessedCommands() const
{
    return m_processedCommands;
}

std::chrono::steady_clock::time_point ServerContext::getStartTime() const
{
    return m_startTime;
}