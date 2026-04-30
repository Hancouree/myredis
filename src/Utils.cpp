#include "../include/Utils.h"
#include <algorithm>

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

        std::string simple(const std::string& str) {
            return "+" + str + "\r\n";
        }
    }

    bool matches(const std::string& key, const std::string& pattern) {
	    size_t n = key.size(), m = pattern.size();
	    size_t k = 0, p = 0;
	    size_t star_k = std::string::npos, star_p = std::string::npos;

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
			    else if (star_p != std::string::npos) {
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
		    else if (star_p != std::string::npos) {
			    p = star_p + 1; k = ++star_k;
		    }
		    else return false;
	    }
	    while (p < m && pattern[p] == '*') ++p;

	    return p == m;
    }

    namespace {
        bool hasWildcards(std::string_view str) {
            return str.find_first_of("*?[") != std::string_view::npos;
        }
    }

    PatternTree::PatternTree() : root(std::make_unique<PatternNode>()) {}

    void PatternTree::add(const std::string& pattern, Session* session) {
        if (pattern.empty() || !session) return;
        addRecursive(root.get(), pattern, pattern, session);
    }

    void PatternTree::del(const std::string& pattern, Session* session)
    {
        if (pattern.empty() || !session) return;
        delRecursive(root.get(), pattern, session);
    }

    std::vector<Session*> PatternTree::findMatches(const std::string& channel) {
        if (channel.empty()) return {};

        std::vector<Session*> result;
        result.reserve(16);

        collectMatches(root.get(), channel, 0, result);

        if (result.size() > 1) {
            std::sort(result.begin(), result.end());
            result.erase(std::unique(result.begin(), result.end()), result.end());
        }

        return result;
    }

    int PatternTree::commonPrefix(const std::string& a, const std::string& b)
    {
        int len = std::min(a.size(), b.size());
        for (int i = 0; i < len; ++i) {
            if (a[i] != b[i]) return i;
        }
        return len;
    }

    void PatternTree::addRecursive(PatternNode* node, const std::string& current,
        const std::string& fullPattern, Session* session) {
        if (current.empty()) {
            node->isEnd = true;
            node->fullPattern = fullPattern;
            if (std::find(node->subscribers.begin(), node->subscribers.end(), session) == node->subscribers.end()) {
                node->subscribers.push_back(session);
            }

            return;
        }

        char firstChar = current[0];

        if (!node->children.contains(firstChar)) {
            auto newNode = std::make_unique<PatternNode>(current, fullPattern);
            newNode->isEnd = true;
            newNode->subscribers.push_back(session);
            node->children[firstChar] = std::move(newNode);
            return;
        }

        PatternNode* child = node->children[firstChar].get();
        int commonLen = commonPrefix(child->label, current);

        if (commonLen == child->label.size()) {
            addRecursive(child, current.substr(commonLen), fullPattern, session);
            return;
        }

        std::string commonPart = child->label.substr(0, commonLen);
        std::string childRemaining = child->label.substr(commonLen);
        std::string currentRemaining = current.substr(commonLen);

        auto splitNode = std::make_unique<PatternNode>(commonPart, "");

        child->label = childRemaining;
        splitNode->children[childRemaining[0]] = std::move(node->children[firstChar]);

        if (!currentRemaining.empty()) {
            auto newLeaf = std::make_unique<PatternNode>(currentRemaining, fullPattern);
            newLeaf->isEnd = true;
            newLeaf->subscribers.push_back(session);
            splitNode->children[currentRemaining[0]] = std::move(newLeaf);
        }
        else {
            splitNode->isEnd = true;
            splitNode->fullPattern = fullPattern;
            splitNode->subscribers.push_back(session);
        }

        node->children[firstChar] = std::move(splitNode);
    }

    bool PatternTree::delRecursive(PatternNode* node, const std::string& current, Session* session)
    {
        if (current.empty()) {
            if (!node->isEnd) return false;

            auto it = std::find(node->subscribers.begin(), node->subscribers.end(), session);
            if (it != node->subscribers.end()) {
                node->subscribers.erase(it);
            }

            if (!node->subscribers.empty()) return false;
            node->isEnd = false;
            node->fullPattern.clear();

            return node->children.empty();
        }

        char firstChar = current[0];
        if (!node->children.contains(firstChar)) return false;

        PatternNode* child = node->children[firstChar].get();

        if (current.find(child->label) != 0) return false;

        bool shouldDeleteChild = delRecursive(child, current.substr(child->label.size()), session);

        if (shouldDeleteChild) {
            node->children.erase(firstChar);

            if (node->children.size() == 1 && !node->isEnd && node != root.get()) {
                auto itNext = node->children.begin();
                auto nextChild = std::move(itNext->second);

                node->label += nextChild->label;
                node->isEnd = nextChild->isEnd;
                node->fullPattern = std::move(nextChild->fullPattern);
                node->subscribers = std::move(nextChild->subscribers);
                node->children = std::move(nextChild->children);

                return false;
            }

            return node->children.empty() && !node->isEnd && node != root.get();
        }

        return false;
    }

    void PatternTree::collectMatches(PatternNode* node, const std::string& channel, size_t pos, std::vector<Session*>& psubscribers) {
        if (node->isEnd) {
            if (matches(channel, node->fullPattern)) {
                psubscribers.insert(psubscribers.end(), node->subscribers.begin(), node->subscribers.end());
            }
        }

        for (auto& [firstChar, child] : node->children) {
            const std::string& label = child->label;

            if (firstChar == '*' || firstChar == '?' || firstChar == '[') {
                collectMatches(child.get(), channel, pos, psubscribers);
            }
            else if (pos < channel.size() && channel[pos] == firstChar) {
                size_t len = label.size();

                if (!hasWildcards(label)) {
                    if (channel.compare(pos, len, label) == 0) {
                        collectMatches(child.get(), channel, pos + len, psubscribers);
                    }
                }
                else {
                    collectMatches(child.get(), channel, pos, psubscribers);
                }
            }
        }
    }
}