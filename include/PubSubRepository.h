#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include "Utils.h"

class Session;

class PubSubRepository
{
public:
    void subscribe(const std::string& channel, Session* session);
    void psubscribe(const std::string& pattern, Session* session);
    int publish(const std::string& channel, const std::string& payload);
    void unsubscribe(const std::string& channel, Session* session);
    void punsubscribe(const std::string& pattern, Session* session);
private:
    std::unordered_map<std::string, std::unordered_set<Session*>> m_subscribers;
    Utils::PatternTree m_patternSubscribers;
};

