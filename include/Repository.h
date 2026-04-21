#pragma once
#include <string>
#include <unordered_map>
#include <map>
#include <optional>
#include <variant>
#include <deque>
#include <chrono>

using String = std::string;
using List = std::deque<std::string>;
using Hash = std::unordered_map<std::string, std::string>;
using RecordValue = std::variant<String, List, Hash>;

struct Record {
	RecordValue value;
	std::optional<std::chrono::steady_clock::time_point> expires_at;
};

class Repository
{
public:
	void performCleanup();
	void set(const std::string& key, const std::string& value);
	bool expires(const std::string& key, int seconds);
	const RecordValue* get(const std::string& key);
	bool del(const std::string& key);
	size_t getMemoryUsed();
	size_t count() const;

	//LIST
	int lpush(const std::string& key, const std::string& value);
	int rpush(const std::string& key, const std::string& value);
	std::optional<String> lpop(const std::string& key);
	std::optional<String> rpop(const std::string& key);
	int llen(const std::string& key);
	std::optional<String> lindex(const std::string& key, int idx);
	List lrange(const std::string& key, int start, int stop);
	int linsert(const std::string& key, const std::string& where, const std::string& pivot, const std::string& value);
	void lset(const std::string& key, int index, const std::string& value);
	void ltrim(const std::string& key, int start, int stop);
	
	//HASH
	int hset(const std::string& key, const std::string& field, const std::string& value);
	std::optional<String> hget(const std::string& key, const std::string& field);
	const Hash* hgetall(const std::string& key);
	bool hdel(const std::string& key, const std::string& field);
	bool hexists(const std::string& key, const std::string& field);
	int hlen(const std::string& key);
private:
	std::unordered_map<std::string, Record> m_data;
	std::multimap<std::chrono::steady_clock::time_point, std::string> m_expiringKeys;
	bool m_isCacheDirty = true;
	size_t m_cachedMemoryUsed = 0;
};
