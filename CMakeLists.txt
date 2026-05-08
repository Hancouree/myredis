cmake_minimum_required(VERSION 3.20)

project(myredis
  VERSION 1.0.0
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(MYREDIS_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

# ---- Dependencies ----
# This project uses Boost.Asio (header-only) but may require Boost.System on some platforms/toolchains.
find_package(Boost 1.70 REQUIRED)

# ---- Target ----
add_executable(myredis
  myredis.cpp
  src/Cleaner.cpp
  src/CommandDocs.cpp
  src/Config.cpp
  src/Handler.cpp
  src/Listener.cpp
  src/Parser.cpp
  src/PubSubRepository.cpp
  src/Registry.cpp
  src/Repository.cpp
  src/ServerContext.cpp
  src/Session.cpp
  src/SessionHandler.cpp
  src/Utils.cpp
)

target_include_directories(myredis
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${Boost_INCLUDE_DIRS}
)

target_compile_features(myredis PRIVATE cxx_std_20)

# Prefer linking Boost::system when available; otherwise fall back to header-only.
if(TARGET Boost::system)
  target_link_libraries(myredis PRIVATE Boost::system)
endif()

if(WIN32)
  # Boost.Asio on Windows uses WinSock.
  target_link_libraries(myredis PRIVATE ws2_32)
endif()

if(UNIX AND NOT APPLE)
  # For threads on Linux/other UNIX.
  set(THREADS_PREFER_PTHREAD_FLAG ON)
  find_package(Threads REQUIRED)
  target_link_libraries(myredis PRIVATE Threads::Threads)
endif()

# ---- Warnings ----
if(MSVC)
  target_compile_options(myredis PRIVATE /W4 /permissive- /Zc:__cplusplus)
  if(MYREDIS_WARNINGS_AS_ERRORS)
    target_compile_options(myredis PRIVATE /WX)
  endif()
else()
  target_compile_options(myredis PRIVATE -Wall -Wextra -Wpedantic)
  if(MYREDIS_WARNINGS_AS_ERRORS)
    target_compile_options(myredis PRIVATE -Werror)
  endif()
endif()

# ---- Runtime convenience ----
# Copy myredis.conf next to the built executable for local runs.
add_custom_command(TARGET myredis POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "${CMAKE_CURRENT_SOURCE_DIR}/myredis.conf"
          "$<TARGET_FILE_DIR:myredis>/myredis.conf"
)

