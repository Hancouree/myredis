#include "../include/CommandDocs.h"
#include <algorithm>

std::unordered_map<std::string, CommandDoc> CommandDocs::m_docs;

void CommandDocs::init()
{
    m_docs["SET"] = { "SET", "Set the string value of a key", "1.0.0", "string", "O(1)", {"key", "value"} };
    m_docs["GET"] = { "GET", "Get the value of a key", "1.0.0", "string", "O(1)", {"key"} };
    m_docs["MSET"] = { "MSET", "Set multiple keys to multiple values", "1.0.0", "string", "O(N)", {"key value [key value ...]"} };
    m_docs["MGET"] = { "MGET", "Get the values of all the given keys", "1.0.0", "string", "O(N)", {"key [key ...]"} };
    m_docs["APPEND"] = { "APPEND", "Append a value to a key", "1.0.0", "string", "O(1)", {"key", "value"} };
    m_docs["STRLEN"] = { "STRLEN", "Get the length of the value stored in a key", "1.0.0", "string", "O(1)", {"key"} };
    m_docs["INCR"] = { "INCR", "Increment the integer value of a key by one", "1.0.0", "string", "O(1)", {"key"} };
    m_docs["INCRBY"] = { "INCRBY", "Increment the integer value of a key by the given amount", "1.0.0", "string", "O(1)", {"key", "increment"} };
    m_docs["DECR"] = { "DECR", "Decrement the integer value of a key by one", "1.0.0", "string", "O(1)", {"key"} };
    m_docs["DECRBY"] = { "DECRBY", "Decrement the integer value of a key by the given amount", "1.0.0", "string", "O(1)", {"key", "decrement"} };

    m_docs["DEL"] = { "DEL", "Delete a key", "1.0.0", "generic", "O(N)", {"key [key ...]"} };
    m_docs["EXISTS"] = { "EXISTS", "Determine if a key exists", "1.0.0", "generic", "O(N)", {"key [key ...]"} };
    m_docs["EXPIRE"] = { "EXPIRE", "Set a key's time to live in seconds", "1.0.0", "generic", "O(1)", {"key", "seconds"} };
    m_docs["TTL"] = { "TTL", "Get the time to live for a key in seconds", "1.0.0", "generic", "O(1)", {"key"} };
    m_docs["PERSIST"] = { "PERSIST", "Remove the expiration from a key", "1.0.0", "generic", "O(1)", {"key"} };
    m_docs["RENAME"] = { "RENAME", "Rename a key", "1.0.0", "generic", "O(1)", {"key", "newkey"} };
    m_docs["TYPE"] = { "TYPE", "Determine the type stored at key", "1.0.0", "generic", "O(1)", {"key"} };
    m_docs["KEYS"] = { "KEYS", "Find all keys matching the given pattern", "1.0.0", "generic", "O(N)", {"pattern"} };

    m_docs["LPUSH"] = { "LPUSH", "Prepend one or more elements to a list", "1.0.0", "list", "O(N)", {"key", "element [element ...]"} };
    m_docs["RPUSH"] = { "RPUSH", "Append an element to a list", "1.0.0", "list", "O(N)", {"key", "element [element ...]"} };
    m_docs["LPOP"] = { "LPOP", "Remove and get the first element in a list", "1.0.0", "list", "O(1)", {"key"} };
    m_docs["RPOP"] = { "RPOP", "Remove and get the last element in a list", "1.0.0", "list", "O(1)", {"key"} };
    m_docs["LLEN"] = { "LLEN", "Get the length of a list", "1.0.0", "list", "O(1)", {"key"} };
    m_docs["LRANGE"] = { "LRANGE", "Get a range of elements from a list", "1.0.0", "list", "O(N)", {"key", "start", "stop"} };
    m_docs["LINDEX"] = { "LINDEX", "Get an element from a list by its index", "1.0.0", "list", "O(N)", {"key", "index"} };
    m_docs["LSET"] = { "LSET", "Set the value of an element in a list by its index", "1.0.0", "list", "O(N)", {"key", "index", "element"} };
    m_docs["LINSERT"] = { "LINSERT", "Insert an element before or after another element in a list", "1.0.0", "list", "O(N)", {"key", "BEFORE|AFTER", "pivot", "element"} };
    m_docs["LTRIM"] = { "LTRIM", "Trim a list to the specified range", "1.0.0", "list", "O(N)", {"key", "start", "stop"} };

    m_docs["HSET"] = { "HSET", "Set the string value of a hash field", "1.0.0", "hash", "O(1)", {"key", "field", "value"} };
    m_docs["HGET"] = { "HGET", "Get the value of a hash field", "1.0.0", "hash", "O(1)", {"key", "field"} };
    m_docs["HGETALL"] = { "HGETALL", "Get all the fields and values in a hash", "1.0.0", "hash", "O(N)", {"key"} };
    m_docs["HDEL"] = { "HDEL", "Delete a hash field", "1.0.0", "hash", "O(1)", {"key", "field"} };
    m_docs["HEXISTS"] = { "HEXISTS", "Determine if a hash field exists", "1.0.0", "hash", "O(1)", {"key", "field"} };
    m_docs["HLEN"] = { "HLEN", "Get the number of fields in a hash", "1.0.0", "hash", "O(1)", {"key"} };
    m_docs["HKEYS"] = { "HKEYS", "Get all the fields in a hash", "1.0.0", "hash", "O(N)", {"key"} };
    m_docs["HVALS"] = { "HVALS", "Get all the values in a hash", "1.0.0", "hash", "O(N)", {"key"} };
    m_docs["HMGET"] = { "HMGET", "Get the values of all the given hash fields", "1.0.0", "hash", "O(N)", {"key", "field [field ...]"} };

    m_docs["SUBSCRIBE"] = { "SUBSCRIBE", "Subscribe to channels", "1.0.0", "pubsub", "O(N)", {"channel [channel ...]"} };
    m_docs["UNSUBSCRIBE"] = { "UNSUBSCRIBE", "Unsubscribe from channels", "1.0.0", "pubsub", "O(N)", {"[channel [channel ...]]"} };
    m_docs["PUBLISH"] = { "PUBLISH", "Post a message to a channel", "1.0.0", "pubsub", "O(N)", {"channel", "message"} };
    m_docs["PSUBSCRIBE"] = { "PSUBSCRIBE", "Subscribe to channels matching patterns", "1.0.0", "pubsub", "O(N)", {"pattern [pattern ...]"} };
    m_docs["PUNSUBSCRIBE"] = { "PUNSUBSCRIBE", "Unsubscribe from patterns", "1.0.0", "pubsub", "O(N)", {"[pattern [pattern ...]]"} };
    m_docs["PUBSUB"] = { "PUBSUB", "Inspect the state of the Pub/Sub subsystem", "1.0.0", "pubsub", "O(N)", {"subcommand [argument ...]"} };

    m_docs["PING"] = { "PING", "Ping the server", "1.0.0", "server", "O(1)", {"[message]"} };
    m_docs["INFO"] = { "INFO", "Get information and statistics about the server", "1.0.0", "server", "O(1)", {} };
    m_docs["COMMAND"] = { "COMMAND", "Get array of Redis command details", "1.0.0", "server", "O(N)", {"[subcommand [argument ...]]"} };
}

const CommandDoc* CommandDocs::get(std::string cmd)
{
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);
	auto it = m_docs.find(cmd);
	return it != m_docs.end() ? &it->second : nullptr;
}

std::deque<std::string> CommandDocs::allNames()
{
	std::deque<std::string> out;
	for (const auto& [name, _] : m_docs) out.push_back(name);
	return out;
}

int CommandDocs::count()
{
	return m_docs.size();
}

std::vector<const CommandDoc*> CommandDocs::lookup(const std::vector<std::string>& cmds)
{
	std::vector<const CommandDoc*> out;
    if (cmds.empty()) {
        for (const auto& [_, doc] : m_docs) {
            out.push_back(&doc);
        }
    }
    else {
        for (const auto& cmd : cmds) {
            const CommandDoc* doc = get(cmd);
            if (doc) out.push_back(doc);
        }
    }

	return out;
}

