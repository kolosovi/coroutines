cmake_minimum_required(VERSION 3.10)

project(coroutines)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
enable_language(CXX ASM)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/bf66935e07825318ae519675d73d0f3e313b3ec6.zip
)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

include_directories(include)
# Target naming convention: target name corresponds to .cpp filename without the
# src/ prefix, with directory separators converted to hyphens. Everything else
# is left as is.
# CMake Error: Cannot determine link language for target "coroutine-switch_stack_arm64.s".
# CMake Error: CMake can not determine linker language for target: coroutine-switch_stack_arm64.s
add_library(coroutine-switch_stack_arm64.s STATIC src/coroutine/switch_stack_arm64.s)
set_target_properties(coroutine-switch_stack_arm64.s PROPERTIES LINKER_LANGUAGE CXX)

add_executable(coroutine-api.test.cpp src/coroutine/api.test.cpp)
target_link_libraries(coroutine-api.test.cpp coroutine-switch_stack_arm64.s gtest_main)

add_library(coroutine-example.cpp STATIC src/coroutine/example.cpp)

add_executable(coroutine-example.test.cpp src/coroutine/example.test.cpp)
target_link_libraries(coroutine-example.test.cpp coroutine-example.cpp gtest_main)
