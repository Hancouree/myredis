#include "../include/Utils.h"

namespace Utils {
    namespace Resp {
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
            return s.has_value() ? bulk(s.value()) : nil();
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

    bool matches(const std::string& key, const std::string& pattern) {
	    size_t n = key.size(), m = pattern.size();
	    size_t k = 0, p = 0;
	    size_t star_k = -1, star_p = -1;

	    while (k < n) {
		    if (p < m && pattern[p] == '[') {
			    size_t end = pattern.find(']', p);
			    if (end == std::string::npos) return false;

			    bool neg = (pattern[p + 1] == '^');
			    size_t start = neg ? p + 2 : p + 1;
			    bool found = false;

			    for (size_t i = start; i < end; ++i) {
				    if (i + 2 < end && pattern[i + 1] == '-') {
					    if (key[k] >= pattern[i] && key[k] <= pattern[i + 2]) { found = true; break; }
					    i += 2;
				    }
				    else if (key[k] == pattern[i]) {
					    found = true; break;
				    }
			    }

			    if (found != neg) {
				    p = end + 1; ++k;
			    }
			    else if (star_p != -1) {
				    p = star_p + 1; k = ++star_k;
			    }
			    else return false;
		    }
		    else if (p < m && (pattern[p] == '?' || (pattern[p] == key[k]))) {
			    ++k; ++p;
		    }
		    else if (p < m && pattern[p] == '*') {
			    star_p = p++; star_k = k;
		    }
		    else if (star_p != -1) {
			    p = star_p + 1; k = ++star_k;
		    }
		    else return false;
	    }
	    while (p < m && pattern[p] == '*') ++p;
	    return p == m;
    }
}