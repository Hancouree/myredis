#pragma once
#include <boost/asio.hpp>
#include <queue>
#include <unordered_set>
#include "ServerContext.h"
#include "Parser.h"

namespace asio = boost::asio;
using tcp = boost::asio::ip::tcp;

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(tcp::socket s, std::shared_ptr<ServerContext> serverCtx);
    ~Session();

    void run() { doRead(); }
    void doWrite(const std::string& msg);

    bool isSubscribed() const { return !m_subscribedChannels.empty() || !m_subscribedPatterns.empty(); }
    int subscribedChannels() const { return m_subscribedChannels.size(); }
    int subscribedPatterns() const { return m_subscribedPatterns.size(); }
    void addChannel(const std::string& channel);
    void addPattern(const std::string& pattern);
    void removeChannel(const std::string& channel);
    void removePattern(const std::string& pattern);
    const std::unordered_set<std::string>& getSubscribedChannels() const { return m_subscribedChannels; }
    const std::unordered_set<std::string>& getSubscribedPatterns() const { return m_subscribedPatterns; }
private:
    void doRead();
    void doWriteNext();
    std::string handleCommand(std::vector<std::string>& args);

    tcp::socket m_socket;
    asio::streambuf m_buffer;
    Parser m_parser;
    std::shared_ptr<ServerContext> m_serverCtx;
    std::queue<std::string> m_writingQueue;
    std::unordered_set<std::string> m_subscribedChannels;
    std::unordered_set<std::string> m_subscribedPatterns;
};

