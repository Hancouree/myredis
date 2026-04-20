#pragma once
#include <unordered_map>
#include <map>
#include <optional>
#include <variant>
#include <deque>
#include <chrono>

struct Record {
	std::variant<std::string, std::deque<std::string>> value;
	std::optional<std::chrono::steady_clock::time_point> expires_at;
};

class Repository
{
public:
	void performCleanup();
	void set(const std::string& key, const std::string& value);
	int lpush(const std::string& key, const std::string& value);
	int rpush(const std::string& key, const std::string& value);
	bool expires(const std::string& key, int seconds);
	std::optional<std::variant<std::string, std::deque<std::string>>> get(const std::string& key);
	bool del(const std::string& key);
	size_t getMemoryUsed();
	size_t count() const;
private:
	std::unordered_map<std::string, Record> m_data;
	std::multimap<std::chrono::steady_clock::time_point, std::string> m_expiringKeys;
	bool m_isCacheDirty = true;
	size_t m_cachedMemoryUsed = 0;
};
