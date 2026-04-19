#include "../include/Repository.h"

void Repository::set(const std::string& key, const std::string& value)
{
	m_data[key] = { value, std::nullopt };
	m_isCacheDirty = true;
}

bool Repository::expires(const std::string& key, int seconds)
{
	if (auto it = m_data.find(key); it != m_data.end()) {
		it->second.expires_at = std::chrono::steady_clock::now() + std::chrono::seconds(seconds);
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
				return "";
			}
		}

		return it->second.value;
	}

	return "";
}

bool Repository::del(const std::string& key)
{
	m_isCacheDirty = true;
	return m_data.erase(key) > 0;
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



