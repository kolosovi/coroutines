cmake_minimum_required(VERSION 3.10)

project(coroutines)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra -Wpedantic -Werror -fsanitize=undefined -fsanitize=address")
enable_language(CXX ASM)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/bf66935e07825318ae519675d73d0f3e313b3ec6.zip
)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

FetchContent_Declare(
    fmt
    GIT_REPOSITORY git@github.com:fmtlib/fmt.git
    GIT_TAG 9.1.0
)
FetchContent_MakeAvailable(fmt)

include_directories(include)
# include_directories(include build/_deps/fmt-src/include)

# Target naming convention: target name corresponds to .cpp filename without the
# src/ prefix, with directory separators converted to hyphens. Everything else
# is left as is.
add_library(coroutine-switch_stack_arm64.s STATIC src/coroutine/switch_stack_arm64.s)
set_target_properties(coroutine-switch_stack_arm64.s PROPERTIES LINKER_LANGUAGE CXX)

add_executable(coroutine-api.test.cpp src/coroutine/api.test.cpp)
target_link_libraries(coroutine-api.test.cpp coroutine-switch_stack_arm64.s gtest_main)

add_library(io-blocking.cpp STATIC src/io/blocking.cpp)
target_link_libraries(io-blocking.cpp PUBLIC fmt)

add_library(io-addrinfo.cpp STATIC src/io/addrinfo.cpp)

add_executable(examples-blocking_fetch_urls.cpp src/examples/blocking_fetch_urls.cpp)
target_link_libraries(examples-blocking_fetch_urls.cpp fmt::fmt io-addrinfo.cpp io-blocking.cpp)
