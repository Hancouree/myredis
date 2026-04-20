#include "../include/Repository.h"

void Repository::performCleanup()
{
	auto now = std::chrono::steady_clock::now();
	while (!m_expiringKeys.empty() && now >= m_expiringKeys.begin()->first) {
		m_data.erase(m_expiringKeys.begin()->second);
		m_expiringKeys.erase(m_expiringKeys.begin());
		m_isCacheDirty = true;
	}
}

void Repository::set(const std::string& key, const std::string& value)
{
	if (auto it = m_data.find(key); it != m_data.end()) {
		if (it->second.expires_at.has_value()) {
			auto range = m_expiringKeys.equal_range(it->second.expires_at.value());
			
			for (auto e_it = range.first; e_it != range.second; ++e_it) {
				if (e_it->second == key) {
					m_expiringKeys.erase(e_it);
					break;
				}
			}
		}
	}

	m_data[key] = { value, std::nullopt };
	m_isCacheDirty = true;
}

int Repository::lpush(const std::string& key, const std::string& value)
{
	if (auto it = m_data.find(key); it != m_data.end()) {
		if (!std::holds_alternative<List>(it->second.value)) {
			throw std::runtime_error("WRONGTYPE");
		}

		auto& l = std::get<List>(it->second.value);
		l.push_front(value);
		m_isCacheDirty = true;
		return l.size();
	}
	else {
		m_data[key] = { List{value}, std::nullopt };
		m_isCacheDirty = true;
		return 1;
	}
}

int Repository::rpush(const std::string& key, const std::string& value)
{
	if (auto it = m_data.find(key); it != m_data.end()) {
		if (!std::holds_alternative<List>(it->second.value)) {
			throw std::runtime_error("WRONGTYPE");
		}

		auto& l = std::get<List>(it->second.value);
		l.push_back(value);
		m_isCacheDirty = true;
		return l.size();
	}
	else {
		m_data[key] = { List{value}, std::nullopt };
		m_isCacheDirty = true;
		return 1;
	}
}

int Repository::hset(const std::string& key, const std::string& field, const std::string& value)
{
	if (auto it = m_data.find(key); it != m_data.end()) {
		if (!std::holds_alternative<Hash>(it->second.value)) {
			throw std::runtime_error("WRONGTYPE");
		}

		auto& h = std::get<Hash>(it->second.value);
		auto [it_h, inserted] = h.insert_or_assign(field, value);
		m_isCacheDirty = true;
		return inserted ? 1 : 0;
	}
	else {
		m_data[key] = { Hash{ { field, value} }, std::nullopt };
		m_isCacheDirty = true;
		return 1;
	}
}

std::optional<String> Repository::hget(const std::string& key, const std::string& field)
{
	if (auto it = m_data.find(key); it != m_data.end()) {
		if (!std::holds_alternative<Hash>(it->second.value)) {
			throw std::runtime_error("WRONGTYPE");
		}

		auto& h = std::get<Hash>(it->second.value);
		if (auto it_h = h.find(field); it_h != h.end()) {
			return it_h->second;
		}

		return std::nullopt;
	}

	return std::nullopt;
}

const Hash* Repository::hgetall(const std::string& key)
{
	if (auto it = m_data.find(key); it != m_data.end()) {
		if (!std::holds_alternative<Hash>(it->second.value)) {
			throw std::runtime_error("WRONGTYPE");
		}

		auto& h = std::get<Hash>(it->second.value);
		return &h;
	}

	return nullptr;
}

bool Repository::hdel(const std::string& key, const std::string& field)
{
	if (auto it = m_data.find(key); it != m_data.end()) {
		if (!std::holds_alternative<Hash>(it->second.value)) {
			throw std::runtime_error("WRONGTYPE");
		}

		auto& h = std::get<Hash>(it->second.value);
		bool erased = h.erase(field) > 0;
		if (erased) { 
			m_isCacheDirty = true; 
			if (h.empty()) m_data.erase(it);
		}
		return erased;
	}

	return false;
}

bool Repository::hexists(const std::string& key, const std::string& field)
{
	if (auto it = m_data.find(key); it != m_data.end()) {
		if (!std::holds_alternative<Hash>(it->second.value)) {
			throw std::runtime_error("WRONGTYPE");
		}

		auto& h = std::get<Hash>(it->second.value);
		return h.contains(field);
	}

	return false;
}

int Repository::hlen(const std::string& key)
{
	if (auto it = m_data.find(key); it != m_data.end()) {
		if (!std::holds_alternative<Hash>(it->second.value)) {
			throw std::runtime_error("WRONGTYPE");
		}

		return std::get<Hash>(it->second.value).size();
	}

	return 0;
}

bool Repository::expires(const std::string& key, int seconds)
{
	if (auto it = m_data.find(key); it != m_data.end()) {
		auto expires_at = std::chrono::steady_clock::now() + std::chrono::seconds(seconds);
		it->second.expires_at = expires_at;
		m_expiringKeys.insert({ expires_at, key });
		return true;
	}

	return false;
}

const RecordValue* Repository::get(const std::string& key)
{
	if (auto it = m_data.find(key); it != m_data.end()) {
		auto now = std::chrono::steady_clock::now();

		if (it->second.expires_at.has_value()) {
			if (now >= it->second.expires_at) {
				auto range = m_expiringKeys.equal_range(it->second.expires_at.value());
				for (auto e_it = range.first; e_it != range.second; ++e_it) {
					if (e_it->second == key) {
						m_expiringKeys.erase(e_it);
						break;
					}
				}

				m_data.erase(it);
				return nullptr;
			}
		}
		return &it->second.value;
	}
	return nullptr;
}

bool Repository::del(const std::string& key)
{
	if (auto it = m_data.find(key); it != m_data.end()) {
		if (it->second.expires_at.has_value()) {
			auto range = m_expiringKeys.equal_range(it->second.expires_at.value());
			
			for (auto e_it = range.first; e_it != range.second; ++e_it) {
				if (e_it->second == key) {
					m_expiringKeys.erase(e_it);
					break;
				}
			}
		}

		m_data.erase(it);
		m_isCacheDirty = true;
		return true;
	}

	return false;
}

size_t Repository::getMemoryUsed()
{
	if (m_isCacheDirty) {
		m_cachedMemoryUsed = 0;
		for (const auto& [key, record] : m_data) {
			m_cachedMemoryUsed += key.size() + sizeof(std::pair<const std::string, Record>) + 24;

			if (auto* s = std::get_if<String>(&record.value)) {
				m_cachedMemoryUsed += s->size();
			}
			else if (auto* l = std::get_if<List>(&record.value)) {
				m_cachedMemoryUsed += sizeof(List);
				for (const auto& item : *l) {
					m_cachedMemoryUsed += item.size();
				}
			}
			else if (auto* h = std::get_if<Hash>(&record.value)) {
				m_cachedMemoryUsed += sizeof(Hash);
				for (const auto& [field, value] : *h) {
					m_cachedMemoryUsed += field.size() + value.size() + 24;
				}
			}
		}

		m_isCacheDirty = false;
	}

	return m_cachedMemoryUsed;
}

size_t Repository::count() const
{
	return m_data.size();
}
