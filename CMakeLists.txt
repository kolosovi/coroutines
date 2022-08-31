cmake_minimum_required(VERSION 3.10)

project(coroutines)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/bf66935e07825318ae519675d73d0f3e313b3ec6.zip
)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

include_directories(include)
# Target naming convention: target name corresponds to .cpp filename without the
# src/ prefix, with directory separators converted to hyphens and the extension
# omitted. Everything else is left as is.
add_executable(coroutine-api.test src/coroutine/api.test.cpp)
target_link_libraries(coroutine-api.test gtest_main)

add_library(coroutine-example STATIC src/coroutine/example.cpp)

add_executable(coroutine-example.test src/coroutine/example.test.cpp)
target_link_libraries(coroutine-example.test coroutine-example gtest_main)
