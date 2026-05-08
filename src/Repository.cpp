#include "../include/Repository.h"
#include "../include/Utils.h"
#include "../include/ServerContext.h"
#include "../include/Config.h"

Repository::Repository(std::shared_ptr<Config> config)
{
	m_memoryUsed = 0;
	m_memoryLimit = config->getInt("maxmemory", 0);
}

void Repository::performCleanup()
{
	auto now = std::chrono::steady_clock::now();
	while (!m_expiringKeys.empty() && now >= m_expiringKeys.begin()->first) {
		const std::string& expiredKey = m_expiringKeys.begin()->second;
		auto it = m_data.find(expiredKey);
		if (it != m_data.end()) {
			size_t freed = 24 + sizeof(std::string) + (expiredKey.capacity() > 15 ? expiredKey.capacity() : 0)
				+ sizeof(RecordValue) + calculateValueMemory(it->second.value);
			m_memoryUsed -= freed;
			m_data.erase(it);
		}
		m_expiringKeys.erase(m_expiringKeys.begin());
	}
}

void Repository::set(const std::string& key, const std::string& value)
{
	size_t incomingSize = 24 + sizeof(std::string) + (key.capacity() > 15 ? key.capacity() : 0) 
		+ sizeof(RecordValue) + calculateValueMemory(value);

	auto it = m_data.find(key);
	bool exists = it != m_data.end();

	size_t freed = 0;
	if (exists) {
		freed = 24 + sizeof(std::string) + (key.capacity() > 15 ? key.capacity() : 0)
			+ sizeof(RecordValue) + calculateValueMemory(it->second.value);
	}

	if (m_memoryLimit > 0 && m_memoryUsed + incomingSize - freed > m_memoryLimit) {
		throw std::runtime_error("OOM command not allowed when used memory > 'maxmemory'");
	}

	if (exists) {
		if (it->second.expires_at.has_value()) {
			dropExpiration(it->second.expires_at.value(), key);
		}

		m_memoryUsed -= freed;
		it->second = { value, std::nullopt };
	}
	else {
		m_data[key] = { value, std::nullopt };
	}

	m_memoryUsed += incomingSize;
}

bool Repository::expires(const std::string& key, int seconds)
{
	if (auto it = m_data.find(key); it != m_data.end()) {
		if (it->second.expires_at.has_value()) {
			dropExpiration(it->second.expires_at.value(), key);
		}

		auto expires_at = std::chrono::steady_clock::now() + std::chrono::seconds(seconds);
		it->second.expires_at = expires_at;
		m_expiringKeys.insert({ expires_at, key });
		return true;
	}

	return false;
}

RecordValue* const Repository::get(const std::string& key)
{
	if (auto it = m_data.find(key); it != m_data.end()) {
		if (isExpired(it->second.expires_at)) {
			size_t freed = 24 + sizeof(std::string) + (key.capacity() > 15 ? key.capacity() : 0)
				+ sizeof(RecordValue) + calculateValueMemory(it->second.value);
			m_memoryUsed -= freed;
			dropExpiration(it->second.expires_at.value(), key);
			m_data.erase(it);
		}
		else {
			return &it->second.value;
		}
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

		size_t freed = 24 + sizeof(std::string) + (k.capacity() > 15 ? k.capacity() : 0)
			+ sizeof(RecordValue) + calculateValueMemory(it->second.value);

		if (it->second.expires_at.has_value()) {
			dropExpiration(it->second.expires_at.value(), k);
		}

		m_memoryUsed -= freed;
		m_data.erase(it);
		++count;
	}

	return count;
}

int Repository::incrBy(const std::string& key, int delta)
{
	int value = 0;
	std::optional<std::chrono::steady_clock::time_point> ttl = std::nullopt;

	size_t freed = 0;
	auto it = m_data.find(key);
	if (it != m_data.end()) {
		if (!std::holds_alternative<String>(it->second.value)) {
			throw std::runtime_error("WRONGTYPE");
		}
		value = std::stoi(std::get<String>(it->second.value));
		ttl = it->second.expires_at;
		freed = 24 + sizeof(std::string) + (key.capacity() > 15 ? key.capacity() : 0)
			+ sizeof(RecordValue) + calculateValueMemory(it->second.value);
	}

	value += delta;
	std::string newStr = std::to_string(value);
	size_t incomingSize = 24 + sizeof(std::string) + (key.capacity() > 15 ? key.capacity() : 0)
		+ sizeof(RecordValue) + calculateValueMemory(newStr);

	if (m_memoryLimit > 0 && m_memoryUsed - freed + incomingSize > m_memoryLimit) {
		throw std::runtime_error("OOM command not allowed when used memory > 'maxmemory'");
	}

	m_memoryUsed -= freed;
	m_data[key] = { newStr, ttl };
	m_memoryUsed += incomingSize;
	return value;
}

int Repository::append(const std::string& key, const std::string& value)
{
	int size = 0;
	auto it = m_data.find(key);
	if (it != m_data.end()) {
		if (!std::holds_alternative<String>(it->second.value)) {
			throw std::runtime_error("WRONGTYPE");
		}
		String& s = std::get<String>(it->second.value);

		size_t freed = s.capacity() > 15 ? s.capacity() : 0;
		s += value;
		size_t incomingSize = s.capacity() > 15 ? s.capacity() : 0;

		if (m_memoryLimit > 0 && m_memoryUsed - freed + incomingSize > m_memoryLimit) {
			s.resize(s.size() - value.size());
			throw std::runtime_error("OOM command not allowed when used memory > 'maxmemory'");
		}

		m_memoryUsed -= freed;
		m_memoryUsed += incomingSize;
		size = static_cast<int>(s.size());
	}
	else {
		size_t incomingSize = 24 + sizeof(std::string) + (key.capacity() > 15 ? key.capacity() : 0)
			+ sizeof(RecordValue) + calculateValueMemory(value);

		if (m_memoryLimit > 0 && m_memoryUsed + incomingSize > m_memoryLimit) {
			throw std::runtime_error("OOM command not allowed when used memory > 'maxmemory'");
		}

		m_data[key] = { value, std::nullopt };
		m_memoryUsed += incomingSize;
		size = static_cast<int>(value.size());
	}

	return size;
}

int Repository::strlen(const std::string& key)
{
	String* s = getTyped<String>(key);
	return s ? s->size() : 0;
}

void Repository::mset(const std::vector<std::string>& args)
{
	for (int i = 0; i < args.size(); i += 2) {
		set(args[i], args[i + 1]);
	}
}

std::vector<const String*> Repository::mget(const std::vector<std::string>& keys)
{
	std::vector<const String*> out;
	for (const auto& k : keys) {
		const RecordValue* v = get(k);
		if (!v) {
			out.push_back(nullptr);
			continue;
		}
		out.push_back(std::get_if<String>(v));
	}
	return out;
}

int Repository::exists(const std::vector<std::string>& keys)
{
	int count = 0;
	for (const auto& k : keys) {
		const RecordValue* v = get(k);
		if (!v) continue;
		++count;
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
	return elapsed < 0 ? 0 : elapsed;
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
		size_t freed = 24 + sizeof(std::string) + (newKey.capacity() > 15 ? newKey.capacity() : 0)
			+ sizeof(RecordValue) + calculateValueMemory(e_it->second.value);
		m_memoryUsed -= freed;
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

int Repository::lpush(const std::string& key, const std::vector<std::string>& values)
{
	size_t incomingSize = 0;

	List* l = getTyped<List>(key);
	if (!l) {
		incomingSize += 24 + sizeof(std::string) + (key.capacity() > 15 ? key.capacity() : 0)
			+ sizeof(RecordValue);

		m_data[key] = { List{}, std::nullopt };
		l = getTyped<List>(key);
	}
	for (const auto& v : values) {
		incomingSize += sizeof(std::string) + calculateValueMemory(v);
		if (m_memoryLimit > 0 && m_memoryUsed + incomingSize > m_memoryLimit) {
			throw std::runtime_error("OOM command not allowed when used memory > 'maxmemory'");
		}

		l->push_front(v);
	}
	m_memoryUsed += incomingSize;
	return l->size();
}

int Repository::rpush(const std::string& key, const std::vector<std::string>& values)
{
	size_t incomingSize = 0;

	List* l = getTyped<List>(key);
	if (!l) {
		incomingSize += 24 + sizeof(std::string) + (key.capacity() > 15 ? key.capacity() : 0)
			+ sizeof(RecordValue);

		m_data[key] = { List{}, std::nullopt };
		l = getTyped<List>(key);
	}
	for (const auto& v : values) {
		incomingSize += sizeof(std::string) + calculateValueMemory(v);
		if (m_memoryLimit > 0 && m_memoryUsed + incomingSize > m_memoryLimit) {
			throw std::runtime_error("OOM command not allowed when used memory > 'maxmemory'");
		}

		l->push_back(v);
	}
	m_memoryUsed += incomingSize;
	return l->size();
}

std::optional<String> Repository::lpop(const std::string& key)
{
	List* l = getTyped<List>(key);
	if (!l || l->empty()) return std::nullopt;

	std::string popped = std::move(l->front());

	size_t freed = sizeof(std::string) + (popped.capacity() > 15 ? popped.capacity() : 0);

	l->pop_front();
	m_memoryUsed -= freed;
	if (l->empty()) del({ key });
	return popped;
}

std::optional<String> Repository::rpop(const std::string& key)
{
	List* l = getTyped<List>(key);
	if (!l || l->empty()) return std::nullopt;

	std::string popped = std::move(l->back());

	size_t freed = sizeof(std::string) + (popped.capacity() > 15 ? popped.capacity() : 0);

	l->pop_back();
	m_memoryUsed -= freed;
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
	if (idx < 0) idx = l->size() + idx;
	if (idx >= 0 && idx < l->size()) return l->at(idx);
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

	size_t incomingSize = sizeof(std::string) + calculateValueMemory(value);
	if (m_memoryLimit > 0 && m_memoryUsed + incomingSize > m_memoryLimit) {
		throw std::runtime_error("OOM command not allowed when used memory > 'maxmemory'");
	}

	if (where == "BEFORE") {
		l->insert(pivot_it, value);
		m_memoryUsed += incomingSize;
	}
	else if (where == "AFTER") {
		l->insert(pivot_it + 1, value);
		m_memoryUsed += incomingSize;
	}

	return l->size();
}

void Repository::lset(const std::string& key, int idx, const std::string& value)
{
	List* l = getTyped<List>(key);
	if (!l) throw std::runtime_error("no such key");

	if (idx < 0) idx = static_cast<int>(l->size()) + idx;
	if (idx < 0 || idx >= static_cast<int>(l->size()))
		throw std::runtime_error("index out of range");

	size_t freed = sizeof(std::string) + calculateValueMemory((*l)[idx]);
	size_t incomingSize = sizeof(std::string) + calculateValueMemory(value);
	if (m_memoryLimit > 0 && m_memoryUsed - freed + incomingSize > m_memoryLimit) {
		throw std::runtime_error("OOM command not allowed when used memory > 'maxmemory'");
	}
	m_memoryUsed -= freed;
	(*l)[idx] = value;
	m_memoryUsed += incomingSize;
}

void Repository::ltrim(const std::string& key, int start, int stop)
{
	List* l = getTyped<List>(key);
	if (!l) return;

	int size = static_cast<int>(l->size());
	if (start < 0) start = size + start;
	if (stop < 0) stop = size + stop;

	start = std::max(start, 0);
	stop = std::min(stop, size - 1);
	if (start > stop) { del({ key }); return; }

	size_t freed = 0;
	for (int i = 0; i < start; ++i)
		freed += sizeof(std::string) + calculateValueMemory((*l)[i]);
	for (int i = stop + 1; i < size; ++i)
		freed += sizeof(std::string) + calculateValueMemory((*l)[i]);
	m_memoryUsed -= freed;

	*l = List(l->begin() + start, l->begin() + stop + 1);
}

int Repository::hset(const std::string& key, const std::string& field, const std::string& value)
{
	Hash* h = getTyped<Hash>(key);
	if (h) {
		auto existing = h->find(field);
		size_t freed = 0;
		if (existing != h->end()) {
			freed = 24 + sizeof(std::pair<std::string, std::string>)
				+ (field.capacity() > 15 ? field.capacity() : 0)
				+ (existing->second.capacity() > 15 ? existing->second.capacity() : 0);
		}
		size_t incomingSize = 24 + sizeof(std::pair<std::string, std::string>)
			+ (field.capacity() > 15 ? field.capacity() : 0)
			+ (value.capacity() > 15 ? value.capacity() : 0);

		if (m_memoryLimit > 0 && m_memoryUsed - freed + incomingSize > m_memoryLimit) {
			throw std::runtime_error("OOM command not allowed when used memory > 'maxmemory'");
		}

		auto [it_h, inserted] = h->insert_or_assign(field, value);
		m_memoryUsed -= freed;
		m_memoryUsed += incomingSize;
		return inserted ? 1 : 0;
	}

	size_t incomingSize = 24 + sizeof(std::string) + (key.capacity() > 15 ? key.capacity() : 0)
		+ sizeof(RecordValue)
		+ 24 + sizeof(std::pair<std::string, std::string>)
		+ (field.capacity() > 15 ? field.capacity() : 0)
		+ (value.capacity() > 15 ? value.capacity() : 0);

	if (m_memoryLimit > 0 && m_memoryUsed + incomingSize > m_memoryLimit) {
		throw std::runtime_error("OOM command not allowed when used memory > 'maxmemory'");
	}

	m_data[key] = { Hash{{field, value}}, std::nullopt };
	m_memoryUsed += incomingSize;
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

	auto it = h->find(field);
	if (it == h->end()) return false;

	size_t freed = 24 + sizeof(std::pair<std::string, std::string>)
		+ (field.capacity() > 15 ? field.capacity() : 0)
		+ (it->second.capacity() > 15 ? it->second.capacity() : 0);
	m_memoryUsed -= freed;

	h->erase(it);
	if (h->empty()) del({ key });
	return true;
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

size_t Repository::calculateValueMemory(const RecordValue& v)
{
	return std::visit([](auto&& arg) -> size_t {
		using T = std::decay_t<decltype(arg)>;

		if constexpr (std::is_same_v<T, std::string>) {
			return (arg.capacity() > 15 ? arg.capacity() : 0);
		}
		else if constexpr (std::is_same_v<T, std::deque<std::string>>) {
			size_t sum = 0;
			for (const auto& s : arg) {
				sum += sizeof(std::string) + (s.capacity() > 15 ? s.capacity() : 0);
			}
			return sum;
		}
		else if constexpr (std::is_same_v<T, std::unordered_map<std::string, std::string>>) {
			size_t sum = 0;
			for (const auto& [k, val] : arg) {
				sum += 24 + sizeof(std::pair<std::string, std::string>) +
					(k.capacity() > 15 ? k.capacity() : 0) +
					(val.capacity() > 15 ? val.capacity() : 0);
			}
			return sum;
		}

		return 0;
	}, v);
}

size_t Repository::getMemoryUsed()
{
	return m_memoryUsed;
}

size_t Repository::count() const
{
	return m_data.size();
}