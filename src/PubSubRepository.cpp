#include "../include/PubSubRepository.h"

//
#include <iostream>


void PubSubRepository::subscribe(const std::string& channel, Session* session)
{
    m_subscribers[channel].insert(session);
}

void PubSubRepository::psubscribe(const std::string& pattern, Session* session)
{
    m_patternSubscribers.add(pattern, session);
}

int PubSubRepository::publish(const std::string& channel, const std::string& payload, PublishCallback callback)
{
    int count = 0;

    if (auto it = m_subscribers.find(channel); it != m_subscribers.end()) {
        for (auto* sub : it->second)
            callback(sub, channel, "", payload);
        count += it->second.size();
    }

    auto patternReceivers = m_patternSubscribers.findMatches(channel);
    for (const auto& [pattern, sub] : patternReceivers) {
        callback(sub, channel, pattern, payload);
    }
    count += patternReceivers.size();

    return count;
}

void PubSubRepository::unsubscribe(const std::string& channel, Session* session)
{
    auto it = m_subscribers.find(channel);
    if (it == m_subscribers.end()) return;

    it->second.erase(session);
    if (it->second.empty()) m_subscribers.erase(it);
}

void PubSubRepository::punsubscribe(const std::string& pattern, Session* session)
{
    m_patternSubscribers.del(pattern, session);
}

List PubSubRepository::pubsub(std::string pattern)
{
    List out;
    for (const auto& [channel, subs] : m_subscribers) {
        if (pattern.empty() || Utils::matches(channel, pattern)) {
            out.push_back(channel);
        }
    }

    return out;
}
