#include "../include/ServerContext.h"
#include "../include/Utils.h"

ServerContext::ServerContext(Config cfg)
    : m_activeConnections(0)
    , m_allConnections(0)
    , m_processedCommands(0)
    , m_config(std::move(cfg))
    , m_repo(m_config)
    , m_pubSubRepo()
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

Config& ServerContext::config()
{
    return m_config;
}

Repository& ServerContext::repo()
{
    return m_repo;
}

PubSubRepository& ServerContext::pubsubRepo()
{
    return m_pubSubRepo;
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