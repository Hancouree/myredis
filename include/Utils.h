#pragma once
#include <string>
#include <deque>
#include <unordered_map>
#include <optional>
#include <memory>

class Session;

namespace Utils {
	namespace Resp {
		std::string integer(int value);
		std::string bulk(const std::string& str);
		std::string nullableBulk(const std::optional<std::string>& s);
		std::string list(const std::deque<std::string>& l);
		std::string hash(const std::unordered_map<std::string, std::string>& h);
		std::string emptyArr();
		std::string nil();
		std::string ok();
		std::string error(const std::string& msg);
		std::string pong();
		std::string simple(const std::string& str);
	}

	bool matches(const std::string& key, const std::string& pattern);

	struct PatternNode {
		std::string label;
		std::string fullPattern;
		std::unordered_map<char, std::unique_ptr<PatternNode>> children;
		std::vector<Session*> subscribers;
		bool isEnd = false;

		PatternNode(std::string l = "", std::string pattern = "") : label(l), fullPattern(pattern) {};
	};

	class PatternTree {
	public:
		PatternTree();
		void add(const std::string& pattern, Session* session);
		void del(const std::string& pattern, Session* session);
		std::vector<Session*> findMatches(const std::string& channel);
	private:
		int commonPrefix(const std::string& a, const std::string& b);
		void addRecursive(PatternNode* node, const std::string& current, const std::string& pattern, Session* session);
		bool delRecursive(PatternNode* node, const std::string& current, Session* session);
		void collectMatches(PatternNode* node, const std::string& channel, size_t pos, std::vector<Session*>& psubscribers);

		std::unique_ptr<PatternNode> root;
	};
}