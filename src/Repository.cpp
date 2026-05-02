#include "../include/Repository.h"
#include "../include/Utils.h"

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
			dropExpiration(it->second.expires_at.value(), key);
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

const RecordValue* Repository::get(const std::string& key)
{
	if (auto it = m_data.find(key); it != m_data.end()) {
		if (isExpired(it->second.expires_at)) {
			dropExpiration(it->second.expires_at.value(), key);
			m_data.erase(it);
			return nullptr;
		}

		return &it->second.value;
	}

	return nullptr;
}

int Repository::del(const std::vector<std::string>& keys)
{
	int count = 0;
	for (const auto& k : keys) {
		auto it = m_data.find(k);
		if (it == m_data.end()) {
			continue;
		}

		if (it->second.expires_at.has_value()) {
			dropExpiration(it->second.expires_at.value(), k);
		}

		m_data.erase(it);
		m_isCacheDirty = true;
		++count;
	}

	return count;
}

int Repository::incrBy(const std::string& key, int delta)
{
	int value = 0;
	std::optional<std::chrono::steady_clock::time_point> ttl = std::nullopt;
	if (auto it = m_data.find(key); it != m_data.end()) {
		if (!std::holds_alternative<String>(it->second.value)) {
			throw std::runtime_error("WRONGTYPE");
		}

		value = std::stoi(std::get<String>(it->second.value));
		ttl = it->second.expires_at;
	}
	value += delta;
	m_data[key] = { std::to_string(value), ttl };
	m_isCacheDirty = true;
	return value;
}

int Repository::append(const std::string& key, const std::string& value)
{
	int size = 0;
	String* s = getTyped<String>(key);
	if (s) {
		*s += value;
		size = s->size();
	}
	else {
		m_data[key] = { value, std::nullopt };
		size = value.size();
	}

	m_isCacheDirty = true;
	return size;
}

int Repository::strlen(const std::string& key)
{
	String* s = getTyped<String>(key);
	return s ? s->size() : 0;
}

std::vector<std::optional<String>> Repository::mget(const std::vector<std::string>& keys)
{
	std::vector<std::optional<String>> out;
	for (const auto& k : keys) {
		auto it = m_data.find(k);
		if (it == m_data.end()) {
			out.push_back(std::nullopt);
			continue;
		}

		if (auto* s = std::get_if<String>(&it->second.value)) {
			out.push_back(*s);
		}
		else {
			out.push_back(std::nullopt);
		}
	}

	return out;
}

int Repository::exists(const std::vector<std::string>& keys)
{
	int count = 0;
	for (const auto& k : keys) {
		if (auto it = m_data.find(k); it != m_data.end()) {
			if (isExpired(it->second.expires_at))
				continue;

			++count;
		}
	}

	return count;
}

int Repository::ttl(const std::string& key)
{
	auto it = m_data.find(key);
	if (it == m_data.end()) return -2;
	if (!it->second.expires_at.has_value()) return -1;

	auto now = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
		it->second.expires_at.value() - now
	).count();
	return elapsed;
}

bool Repository::persist(const std::string& key)
{
	auto it = m_data.find(key);
	if (it == m_data.end()) return false;
	if (!it->second.expires_at.has_value()) return false;
	
	dropExpiration(it->second.expires_at.value(), key);
	it->second.expires_at = std::nullopt;

	return true;
}

void Repository::rename(const std::string& key, const std::string& newKey)
{
	auto it = m_data.find(key);
	if (it == m_data.end()) {
		throw std::runtime_error("no such key");
	}

	if (auto e_it = m_data.find(newKey); e_it != m_data.end()) {
		if (e_it->second.expires_at.has_value()) {
			dropExpiration(e_it->second.expires_at.value(), newKey);
		}
		m_data.erase(e_it);
	}

	RecordValue value = std::move(it->second.value);
	auto expires_at = it->second.expires_at;

	m_data.erase(it);
	if (expires_at.has_value()) {
		dropExpiration(expires_at.value(), key);
		m_expiringKeys.insert({ expires_at.value(), newKey });
	}

	m_data[newKey] = { std::move(value), expires_at };
	m_isCacheDirty = true;
}

List Repository::keys(const std::string& pattern)
{
	List l;
	for (auto it = m_data.begin(); it != m_data.end(); ++it) {
		std::string key = it->first;

		if (Utils::matches(key, pattern) && !isExpired(it->second.expires_at)) {
			l.push_back(key);
		}
	}

	return l;
}

int Repository::lpush(const std::string& key, const std::string& value)
{
	int size = 0;
	List* l = getTyped<List>(key);

	if (l) {
		l->push_front(value);
		size = l->size();
	}
	else {
		m_data[key] = { List{value}, std::nullopt };
		size = 1;
	}

	m_isCacheDirty = true;
	return size;
}

int Repository::rpush(const std::string& key, const std::string& value)
{
	int size = 0;
	List* l = getTyped<List>(key);

	if (l) {
		l->push_back(value);
		size = l->size();
	}
	else {
		m_data[key] = { List{value}, std::nullopt };
		size = 1;
	}

	m_isCacheDirty = true;
	return size;
}

std::optional<String> Repository::lpop(const std::string& key)
{
	List* l = getTyped<List>(key);
	if (!l || l->empty()) return std::nullopt;

	std::string popped = l->front();
	l->pop_front();
	m_isCacheDirty = true;
	if (l->empty()) del({ key });
	return popped;
}

std::optional<String> Repository::rpop(const std::string& key)
{
	List* l = getTyped<List>(key);
	if (!l || l->empty()) return std::nullopt;

	std::string popped = l->back();
	l->pop_back();
	m_isCacheDirty = true;
	if (l->empty()) del({ key });
	return popped;
}

int Repository::llen(const std::string& key)
{
	List* l = getTyped<List>(key);
	return l ? l->size() : 0;
}

std::optional<String> Repository::lindex(const std::string& key, int idx)
{
	List* l = getTyped<List>(key);
	if (!l) return std::nullopt;
	if (idx < 0) idx = (int)l->size() + idx;
	if (idx >= 0 && idx < (int)l->size()) return l->at(idx);
	return std::nullopt;
}

List Repository::lrange(const std::string& key, int start, int stop)
{
	List* l = getTyped<List>(key);
	if (!l) return {};

	int size = l->size();
	if (start < 0) start = size + start;
	if (stop < 0)  stop = size + stop;
	start = std::max(start, 0);
	stop = std::min(stop, size - 1); 
	if (start > stop) return {};
	return List(l->begin() + start, l->begin() + stop + 1);
}

int Repository::linsert(const std::string& key, const std::string& where, const std::string& pivot, const std::string& value)
{
	List* l = getTyped<List>(key);
	if (!l) return 0;

	auto pivot_it = std::find(l->begin(), l->end(), pivot);
	if (pivot_it == l->end()) return -1;

	if (where == "BEFORE") {
		l->insert(pivot_it, value);
		m_isCacheDirty = true;
	}
	else if (where == "AFTER") {
		l->insert(pivot_it + 1, value);
		m_isCacheDirty = true;
	}

	return l->size();
}

void Repository::lset(const std::string& key, int idx, const std::string& value)
{
	List* l = getTyped<List>(key);
	if (!l) throw std::runtime_error("no such key");

	if (idx < 0) idx = l->size() + idx;
	if (idx < 0 || idx >= l->size()) 
		throw std::runtime_error("index out of range");

	(*l)[idx] = value;
	m_isCacheDirty = true;
}

void Repository::ltrim(const std::string& key, int start, int stop)
{
	List* l = getTyped<List>(key);
	if (!l) return;

	if (start < 0) start = l->size() + start;
	if (stop < 0) stop = l->size() + stop;

	start = std::max(start, 0);
	stop = std::min(stop, (int)l->size() - 1);
	if (start > stop) { del({ key }); return; }

	*l = List(l->begin() + start, l->begin() + stop + 1);
}

int Repository::hset(const std::string& key, const std::string& field, const std::string& value)
{
	Hash* h = getTyped<Hash>(key);
	if (h) {
		auto [it_h, inserted] = h->insert_or_assign(field, value);
		m_isCacheDirty = true;
		return inserted ? 1 : 0;
	}
	m_data[key] = { Hash{{field, value}}, std::nullopt };
	m_isCacheDirty = true;
	return 1;
}

std::optional<String> Repository::hget(const std::string& key, const std::string& field)
{
	Hash* h = getTyped<Hash>(key);
	if (!h) return std::nullopt;
	auto it = h->find(field);
	return it != h->end() ? std::optional<String>(it->second) : std::nullopt;
}

const Hash* Repository::hgetall(const std::string& key)
{
	return getTyped<Hash>(key);
}

bool Repository::hdel(const std::string& key, const std::string& field)
{
	Hash* h = getTyped<Hash>(key);
	if (!h) return false;
	bool erased = h->erase(field) > 0;
	if (erased) {
		m_isCacheDirty = true;
		if (h->empty()) del({ key });
	}
	return erased;
}

bool Repository::hexists(const std::string& key, const std::string& field)
{
	Hash* h = getTyped<Hash>(key);
	return h ? h->contains(field) : false;
}

int Repository::hlen(const std::string& key)
{
	Hash* h = getTyped<Hash>(key);
	return h ? (int)h->size() : 0;
}

List Repository::hkeys(const std::string& key)
{
	Hash* h = getTyped<Hash>(key);
	if (!h) return {};
	List l;
	for (const auto& [k, _] : *h) l.push_back(k);
	return l;
}

List Repository::hvals(const std::string& key)
{
	Hash* h = getTyped<Hash>(key);
	if (!h) return {};
	List l;
	for (const auto& [_, v] : *h) l.push_back(v);
	return l;
}

std::vector<std::optional<String>> Repository::hmget(const std::string& key, const std::vector<std::string>& fields)
{
	Hash* h = getTyped<Hash>(key);
	std::vector<std::optional<String>> results;
	for (const auto& f : fields) {
		if (!h) { results.push_back(std::nullopt); continue; }
		auto it = h->find(f);
		results.push_back(it != h->end() ? std::optional<String>(it->second) : std::nullopt);
	}
	return results;
}

void Repository::dropExpiration(const std::chrono::steady_clock::time_point& tp, const std::string& key)
{
	auto range = m_expiringKeys.equal_range(tp);
	for (auto it = range.first; it != range.second; ++it) {
		if (it->second == key) {
			m_expiringKeys.erase(it);
			break;
		}
	}
}

bool Repository::isExpired(const std::optional<std::chrono::steady_clock::time_point>& tp)
{
	if (!tp.has_value()) return false;
	auto now = std::chrono::steady_clock::now();
	return now >= tp;
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