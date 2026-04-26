# myredis

A Redis-compatible in-memory data store written in C++, built with Boost.Asio for asynchronous networking.

## Requirements

- Windows
- CMake 3.15+
- Boost (Asio)
- C++20 compiler (MSVC recommended)

## Building

```bash
git clone https://github.com/yourname/myredis.git
cd myredis
cmake -B build
cmake --build build --config Release
```

## Running

```bash
./build/Release/myredis.exe
```

Server listens on `127.0.0.1:5050`. Connect with any Redis-compatible client:

```bash
redis-cli -p 5050
```

## Supported Commands

### Strings
| Command | Description |
|---|---|
| `SET key value` | Set a string value |
| `GET key` | Get a string value |
| `MSET key value [key value ...]` | Set multiple keys |
| `MGET key [key ...]` | Get multiple keys |
| `APPEND key value` | Append to a string |
| `STRLEN key` | Get string length |
| `INCR key` | Increment integer value by 1 |
| `INCRBY key increment` | Increment by a given amount |
| `DECR key` | Decrement integer value by 1 |
| `DECRBY key decrement` | Decrement by a given amount |

### Lists
| Command | Description |
|---|---|
| `LPUSH key element` | Prepend element to list |
| `RPUSH key element` | Append element to list |
| `LPOP key` | Remove and return first element |
| `RPOP key` | Remove and return last element |
| `LLEN key` | Get list length |
| `LRANGE key start stop` | Get a range of elements |
| `LINDEX key index` | Get element by index |
| `LSET key index element` | Set element at index |
| `LINSERT key BEFORE\|AFTER pivot element` | Insert element relative to pivot |
| `LTRIM key start stop` | Trim list to a range |

### Hashes
| Command | Description |
|---|---|
| `HSET key field value` | Set a hash field |
| `HGET key field` | Get a hash field |
| `HMGET key field [field ...]` | Get multiple hash fields |
| `HGETALL key` | Get all fields and values |
| `HDEL key field` | Delete a hash field |
| `HEXISTS key field` | Check if field exists |
| `HLEN key` | Get number of fields |
| `HKEYS key` | Get all field names |
| `HVALS key` | Get all field values |

### Generic
| Command | Description |
|---|---|
| `DEL key` | Delete a key |
| `EXISTS key [key ...]` | Check if keys exist |
| `EXPIRE key seconds` | Set TTL in seconds |
| `TTL key` | Get remaining TTL |
| `PERSIST key` | Remove TTL |
| `RENAME key newkey` | Rename a key |
| `TYPE key` | Get value type |
| `KEYS pattern` | Find keys matching pattern |

### Pub/Sub
| Command | Description |
|---|---|
| `SUBSCRIBE channel` | Subscribe to a channel |
| `UNSUBSCRIBE [channel]` | Unsubscribe from a channel or all |
| `PUBLISH channel message` | Publish a message to a channel |

### Server
| Command | Description |
|---|---|
| `PING [message]` | Ping the server |
| `INFO` | Get server statistics |
| `COMMAND DOCS [command ...]` | Get command documentation |

## Architecture

- **Boost.Asio** — asynchronous TCP networking, single-threaded event loop
- **Registry** — maps command names to handlers
- **Repository** — in-memory key-value store with TTL support
- **Session** — per-client connection, handles RESP protocol parsing and pub/sub state
- **Cleaner** — background timer that evicts expired keys every second
