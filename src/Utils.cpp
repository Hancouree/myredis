#include "../include/Utils.h"

namespace Utils {
    std::string integer(int value)
    {
        return ":" + std::to_string(value) + "\r\n";
    }

    std::string bulk(const std::string& str)
    {
        return "$" + std::to_string(str.size()) + "\r\n" + str + "\r\n";
    }

    std::string nullableBulk(const std::optional<std::string>& s)
    {
        return s.has_value() ? Utils::bulk(s.value()) : Utils::nil();
    }

    std::string list(const std::deque<std::string>& l)
    {
        std::string out = "*" + std::to_string(l.size()) + "\r\n";
        for (const auto& i : l) out += bulk(i);
        return out;
    }

    std::string hash(const std::unordered_map<std::string, std::string>& h)
    {
        std::string out = "*" + std::to_string(h.size() * 2) + "\r\n";
        for (const auto& [key, value] : h) {
            out += bulk(key);
            out += bulk(value);
        }

        return out;
    }

    std::string emptyArr()
    {
        return "*0\r\n";
    }

    std::string nil()
    {
        return "$-1\r\n";
    }

    std::string ok()
    {
        return "+OK\r\n";
    }

    std::string error(const std::string& msg)
    {
        return "-ERR " + msg + "\r\n";
    }

    std::string pong()
    {
        return "+PONG\r\n";
    }

}