#pragma once
#include <memory>
#include "Repository.h"

class ServerContext {
public:
    std::shared_ptr<Repository> m_repo;

    ServerContext();

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

