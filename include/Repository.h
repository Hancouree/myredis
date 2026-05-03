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
	int del(const std::vector<std::string>& keys);
	int incrBy(const std::string& key, int delta = 1);
	int append(const std::string& key, const std::string& value);
	int strlen(const std::string& key);
	void mset(const std::vector<std::string>& args);
	std::vector<const String*> mget(const std::vector<std::string>& keys);
	int exists(const std::vector<std::string>& key);
	int ttl(const std::string& key);
	bool persist(const std::string& key);
	void rename(const std::string& key, const std::string& newKey);
	List keys(const std::string& pattern);

	//LIST
	int lpush(const std::vector<std::string>& args);
	int rpush(const std::vector<std::string>& args);
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
	List hkeys(const std::string& key);
	List hvals(const std::string& key);
	std::vector<std::optional<String>> hmget(const std::string& key, const std::vector<std::string>& fields);

	size_t getMemoryUsed();
	size_t count() const;
private:
	void dropExpiration(const std::chrono::steady_clock::time_point& tp, const std::string& key);
	bool isExpired(const std::optional<std::chrono::steady_clock::time_point>& tp);
	
	template <typename T>
	T* getTyped(const std::string& key) {
		auto it = m_data.find(key);
		if (it == m_data.end()) return nullptr;
		if (!std::holds_alternative<T>(it->second.value)) {
			throw std::runtime_error("WRONGTYPE");
		}

		return &std::get<T>(it->second.value);
	}

	std::unordered_map<std::string, Record> m_data;
	std::multimap<std::chrono::steady_clock::time_point, std::string> m_expiringKeys;
	bool m_isCacheDirty = true;
	size_t m_cachedMemoryUsed = 0;
};
