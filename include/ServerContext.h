#pragma once
#include <memory>
#include <unordered_set>
#include <atomic>
#include "Repository.h"
#include "PubSubRepository.h"

class Session;
class Config;

class ServerContext {
public:
    std::shared_ptr<Config> m_config;
    std::shared_ptr<Repository> m_repo;
    std::shared_ptr<PubSubRepository> m_pubSubRepo;

    ServerContext(std::shared_ptr<Config> cfg);

    void incrementConnections();
    void decrementConnections();
    void incrementProcessedCommands();

    int getConnections() const;
    int getAllConnections() const;
    int getAllProcessedCommands() const;
    std::chrono::steady_clock::time_point getStartTime() const;
private:
    std::atomic<int> m_activeConnections;
    std::atomic<int> m_allConnections;
    std::atomic<int> m_processedCommands;
    std::chrono::steady_clock::time_point m_startTime;
};

