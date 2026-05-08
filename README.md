# myredis

`myredis` is a lightweight, Redis-compatible in-memory data store written in C++20.

- **Protocol**: RESP (works with `redis-cli` and most Redis clients for the supported commands)
- **Networking**: Boost.Asio async TCP
- **Data types**: strings, lists, hashes
- **Extras**: key TTL, basic memory limit, pub/sub, `INFO`, `COMMAND ...`

> This is not a full Redis replacement. The goal is a small, readable codebase with a useful subset of Redis behavior.

## Quick start

Build and run (any OS with CMake + a C++20 compiler):

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/myredis
```

Connect with `redis-cli` (default port is configured in `myredis.conf`):

```bash
redis-cli -p 6239
PING
SET a 1
INCR a
GET a
```

## Configuration (`myredis.conf`)

The server reads `myredis.conf` from the **current working directory**.
When you build with CMake, the file is automatically copied next to the executable for convenience.

Supported options (keys are case-insensitive in the config parser):

- **`bind`**: IP address to listen on (default: `0.0.0.0`)
- **`port`**: TCP port (default: `6239`)
- **`tcp-backlog`**: listen backlog (default: `511`)
- **`timeout`**: idle client timeout in seconds (default: `300`)
- **`subscribed_timeout`**: timeout in seconds while in Pub/Sub mode (default: `3600`)
- **`maxclients`**: max active clients (default: `100`)
- **`maxmemory`**: memory limit in bytes, `0` disables the limit (default: `0`)
- **`hz`**: cleanup frequency (Cleaner ticks per second), default: `10`

Security note:

- **`bind 0.0.0.0` exposes the server to your network**. For local development prefer `127.0.0.1`.

## Building

### Windows (Visual Studio / MSVC)

Prereqs:

- Visual Studio 2022 (or newer) with C++ workload
- CMake 3.20+
- Boost (headers; `boost/asio.hpp` is required)

Build:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
.\build\Release\myredis.exe
```

### Linux (GCC or Clang)

Install deps (Debian/Ubuntu example):

```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake libboost-all-dev
```

Build and run:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/myredis
```

### macOS (Apple Clang)

Install deps (Homebrew):

```bash
brew install cmake boost
```

Build:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/myredis
```

## Running

By default `myredis` reads the `bind`/`port` from `myredis.conf`.
To run multiple instances, copy the config and change `port`:

```bash
cp myredis.conf myredis-2.conf
# edit myredis-2.conf: port 6240
./build/myredis   # run from a directory that contains myredis.conf
```

If you want to use Redis tooling:

```bash
redis-cli -h 127.0.0.1 -p 6239
```

## Supported commands

Command handling is registered in `include/Registry.h` + `src/Registry.cpp`, and documentation lives in `include/CommandDocs.h` + `src/CommandDocs.cpp`.

### Strings

- **`SET key value`**
- **`GET key`**
- **`MSET key value [key value ...]`**
- **`MGET key [key ...]`**
- **`APPEND key value`**
- **`STRLEN key`**
- **`INCR key`**
- **`INCRBY key increment`**
- **`DECR key`**
- **`DECRBY key decrement`**

### Lists

- **`LPUSH key element [element ...]`**
- **`RPUSH key element [element ...]`**
- **`LPOP key`**
- **`RPOP key`**
- **`LLEN key`**
- **`LRANGE key start stop`**
- **`LINDEX key index`**
- **`LSET key index element`**
- **`LINSERT key BEFORE|AFTER pivot element`**
- **`LTRIM key start stop`**

### Hashes

- **`HSET key field value`**
- **`HGET key field`**
- **`HMGET key field [field ...]`**
- **`HGETALL key`**
- **`HDEL key field`**
- **`HEXISTS key field`**
- **`HLEN key`**
- **`HKEYS key`**
- **`HVALS key`**

### Generic / keys

- **`DEL key [key ...]`**
- **`EXISTS key [key ...]`**
- **`EXPIRE key seconds`**
- **`TTL key`**
- **`PERSIST key`**
- **`RENAME key newkey`**
- **`TYPE key`**
- **`KEYS pattern`**

### Pub/Sub

- **`SUBSCRIBE channel [channel ...]`**
- **`UNSUBSCRIBE [channel [channel ...]]`**
- **`PSUBSCRIBE pattern [pattern ...]`**
- **`PUNSUBSCRIBE [pattern [pattern ...]]`**
- **`PUBLISH channel message`**
- **`PUBSUB CHANNELS [pattern]`**

Notes:

- When a client is in Pub/Sub mode, only subscription commands plus `PING`/`QUIT` are accepted.

### Server / introspection

- **`PING [message]`**
- **`INFO`**
- **`COMMAND`** subcommands:
  - `COMMAND` (returns count)
  - `COMMAND COUNT`
  - `COMMAND LIST`
  - `COMMAND DOCS [command ...]`

## Semantics & limitations

- **RESP parsing**: implemented by `Parser` (`include/Parser.h`, `src/Parser.cpp`).
- **TTL**:
  - `EXPIRE` stores expiration timestamps
  - `Cleaner` periodically evicts expired keys based on `hz`
  - reads (`GET`, etc.) also drop expired keys on access
- **Memory limit** (`maxmemory`):
  - approximate accounting is maintained in `Repository`
  - when the limit is exceeded, write-like commands return an OOM error
- **Single-threaded**: the event loop runs in one `asio::io_context` thread (see `myredis.cpp`).

## Project layout

- **`myredis.cpp`**: entry point (creates config, context, listener, cleaner)
- **`include/`**: headers
- **`src/`**: implementation
- **`myredis.conf`**: runtime configuration

