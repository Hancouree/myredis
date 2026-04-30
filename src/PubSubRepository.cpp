#include "../include/PubSubRepository.h"
#include "../include/Session.h"

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

int PubSubRepository::publish(const std::string& channel, const std::string& payload)
{
    auto it = m_subscribers.find(channel);
    if (it == m_subscribers.end()) return 0;

    auto& directReceivers = it->second;
    std::string message = Utils::Resp::list({ "message", channel, payload });
    for (const auto& sub : directReceivers) {
        sub->doWrite(message);
    }

    auto patternReceivers = m_patternSubscribers.findMatches(channel);
    for (const auto& [pattern, sub] : patternReceivers) {
        std::string pmessage = Utils::Resp::list({ "pmessage", pattern, payload });
        sub->doWrite(pmessage);
    }
    
    return directReceivers.size();
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
