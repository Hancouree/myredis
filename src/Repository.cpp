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
			auto [range_start, range_end] = m_expiringKeys.equal_range(it->second.expires_at.value());
			
			for (auto it = range_start; it != range_end; ++it) {
				if (it->second == key) {
					m_expiringKeys.erase(it);
					break;
				}
			}
		}
	}

	m_data[key] = { value, std::nullopt };
	m_isCacheDirty = true;
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

std::string Repository::get(const std::string& key)
{
	if (auto it = m_data.find(key); it != m_data.end()) {
		auto now = std::chrono::steady_clock::now();
		
		if (it->second.expires_at.has_value()) {
			if (now >= it->second.expires_at) {
				m_data.erase(it);
				auto [range_start, range_end] = m_expiringKeys.equal_range(it->second.expires_at.value());
				for (auto it = range_start; it != range_end; ++it) {
					if (it->second == key) {
						m_expiringKeys.erase(it);
						break;
					}
				}

				return "";
			}
		}

		return it->second.value;
	}

	return "";
}

bool Repository::del(const std::string& key)
{
	if (auto it = m_data.find(key); it != m_data.end()) {
		if (it->second.expires_at.has_value()) {
			auto [range_start, range_end] = m_expiringKeys.equal_range(it->second.expires_at.value());
			
			for (auto it = range_start; it != range_end; ++it) {
				if (it->second == key) {
					m_expiringKeys.erase(it);
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
			m_cachedMemoryUsed += key.size() + record.value.size();
			m_cachedMemoryUsed += sizeof(std::pair<const std::string, Record>) + 24;
		}

		m_isCacheDirty = false;
	}

	return m_cachedMemoryUsed;
}

size_t Repository::count() const
{
	return m_data.size();
}
