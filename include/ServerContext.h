#pragma once
#include <memory>
#include <unordered_set>
#include <atomic>
#include <chrono>
#include "Repository.h"
#include "PubSubRepository.h"
#include "Config.h"

class ServerContext {
public:
    ServerContext(Config cfg);

    void incrementConnections();
    void decrementConnections();
    void incrementProcessedCommands();

    Config& config();
    Repository& repo();
    PubSubRepository& pubsubRepo();

    int getConnections() const;
    int getAllConnections() const;
    int getAllProcessedCommands() const;
    std::chrono::steady_clock::time_point getStartTime() const;
private:
    std::atomic<int> m_activeConnections;
    std::atomic<int> m_allConnections;
    std::atomic<int> m_processedCommands;
    std::chrono::steady_clock::time_point m_startTime;

    Config m_config;
    Repository m_repo;
    PubSubRepository m_pubSubRepo;
};

