#pragma once
#include <string>
#include <unordered_map>
#include <functional>
#include <unordered_set>
#include "Utils.h"

class Session;

using PublishCallback = std::function<void(Session*, const std::string&, const std::string&, const std::string&)>;
using List = std::deque<std::string>;
using Hash = std::unordered_map<std::string, std::string>;

class PubSubRepository
{
public:
    void subscribe(const std::string& channel, Session* session);
    void psubscribe(const std::string& pattern, Session* session);
    int publish(const std::string& channel, const std::string& payload, PublishCallback callback);
    void unsubscribe(const std::string& channel, Session* session);
    void punsubscribe(const std::string& pattern, Session* session);
    List channels(std::string pattern = "");
    Hash numsub(const std::vector<std::string>& channels);
    int numpat();
private:
    std::unordered_map<std::string, std::unordered_set<Session*>> m_subscribers;
    Utils::PatternTree m_patternSubscribers;
};

