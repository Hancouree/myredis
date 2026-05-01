#include "../include/SessionHandler.h"
#include "../include/Utils.h"
#include "../include/Session.h"

std::string SubscribeHandler::execute(
    const std::vector<std::string>& args, 
    std::shared_ptr<ServerContext>& serverCtx, 
    Session* session
) {
    if (args.size() < 2) {
        return Utils::Resp::error("wrong number of arguments for SUBSCRIBE");
    }

    std::vector<std::string> channelsToSubscribe = { args.begin() + 1, args.end() };
    std::string response = "";
    for (const auto& c : channelsToSubscribe) {
        serverCtx->m_pubSubRepo->subscribe(c, session);
        session->addChannel(c);

        response += "*3\r\n" +
                    Utils::Resp::bulk("subscribe") +
                    Utils::Resp::bulk(c) +
                    Utils::Resp::integer(session->subscribedChannels() + session->subscribedPatterns());
    }

    return response;
}

std::string PublishHandler::execute(
    const std::vector<std::string>& args, 
    std::shared_ptr<ServerContext>& serverCtx, 
    Session* session
) {
    if (args.size() < 3) {
        return Utils::Resp::error("wrong number of arguments for PUBLISH");
    }

    std::string channel = args[1];
    int count = serverCtx->m_pubSubRepo->publish(channel, args[2], 
        [](Session* sub, const std::string& channel, const std::string& pattern, const std::string& payload) {
            std::string formattedResp;
            if (pattern.empty()) {
                formattedResp = Utils::Resp::list({ "message", channel,  payload });
            }
            else {
                formattedResp = Utils::Resp::list({ "pmessage", pattern, channel, payload });
            }

            sub->doWrite(formattedResp);
        }
    );

    return Utils::Resp::integer(count);
}

std::string PSubscribeHandler::execute(
    const std::vector<std::string>& args, 
    std::shared_ptr<ServerContext>& serverCtx, 
    Session* session
) {
    if (args.size() < 2) {
        return Utils::Resp::error("wrong number of arguments for PSUBSCRIBE");
    }

    std::vector<std::string> patternsToSubscribe = { args.begin() + 1, args.end() };
    std::string response = "";
    for (const auto& p : patternsToSubscribe) {
        serverCtx->m_pubSubRepo->psubscribe(p, session);
        session->addPattern(p);

        response += "*3\r\n" +
            Utils::Resp::bulk("psubscribe") +
            Utils::Resp::bulk(p) +
            Utils::Resp::integer(session->subscribedChannels() + session->subscribedPatterns());
    }

    return response;
}

std::string UnsubscribeHandler::execute(
    const std::vector<std::string>& args,
    std::shared_ptr<ServerContext>& serverCtx,
    Session* session
) {
    std::vector<std::string> channelsToRemove;

    if (args.size() < 2) {
        const auto& channels = session->getSubscribedChannels();
        channelsToRemove.assign(channels.begin(), channels.end());
    }
    else {
        channelsToRemove.assign(args.begin() + 1, args.end());
    }

    std::string response = "";
    for (const auto& channel : channelsToRemove) {
        serverCtx->m_pubSubRepo->unsubscribe(channel, session);
        session->removeChannel(channel);

        response += "*3\r\n" +
            Utils::Resp::bulk("unsubscribe") +
            Utils::Resp::bulk(channel) +
            Utils::Resp::integer(session->subscribedChannels() + session->subscribedPatterns());
    }

    if (response.empty()) {
        response = "*3\r\n" +
            Utils::Resp::bulk("unsubscribe") +
            Utils::Resp::nil() +
            Utils::Resp::integer(0);
    }

    return response;
}

std::string PUnsubscribeHandler::execute(
    const std::vector<std::string>& args, 
    std::shared_ptr<ServerContext>& serverCtx, 
    Session* session
) {
    std::vector<std::string> patternsToRemove;

    if (args.size() < 2) {
        const auto& patterns = session->getSubscribedPatterns();
        patternsToRemove.assign(patterns.begin(), patterns.end());
    }
    else {
        patternsToRemove.assign(args.begin() + 1, args.end());
    }

    std::string response = "";
    for (const auto& pattern : patternsToRemove) {
        serverCtx->m_pubSubRepo->punsubscribe(pattern, session);
        session->removePattern(pattern);

        response += "*3\r\n" +
            Utils::Resp::bulk("punsubscribe") +
            Utils::Resp::bulk(pattern) +
            Utils::Resp::integer(session->subscribedChannels() + session->subscribedPatterns()); 
    }

    if (response.empty()) {
        response = "*3\r\n" +
            Utils::Resp::bulk("punsubscribe") +
            Utils::Resp::nil() +
            Utils::Resp::integer(0);
    }

    return response;
}

std::string PubSubChannelsHandler::execute(
    const std::vector<std::string>& args, 
    std::shared_ptr<ServerContext>& serverCtx, 
    Session* session
) { 
    if (args.size() < 2) {
        return Utils::Resp::error("wrong number of arguments for PSUBSCRIBE");
    }

    std::string subCommand = args[1];
    std::transform(subCommand.begin(), subCommand.end(), subCommand.begin(), ::toupper);
    if (subCommand != "CHANNELS") {
        return Utils::Resp::error("unknown command");
    }

    return Utils::Resp::list(serverCtx->m_pubSubRepo->pubsub(args.size() < 3 ? "" : args[2]));
}
