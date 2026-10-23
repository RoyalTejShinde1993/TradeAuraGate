
cmake_minimum_required(VERSION 3.14)

project(TradeAuraGate VERSION 1.0 LANGUAGES CXX)

# C++20 configuration
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Default to Release
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

include(FetchContent)

# CPR: REST API client
set(CPR_USE_SYSTEM_CURL OFF CACHE BOOL "" FORCE)
set(CPR_BUILD_TESTS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    cpr
    GIT_REPOSITORY https://github.com/libcpr/cpr.git
    GIT_TAG 1.10.5
)
FetchContent_MakeAvailable(cpr)

# simdjson: JSON parsing
FetchContent_Declare(
    simdjson
    GIT_REPOSITORY https://github.com/simdjson/simdjson.git
    GIT_TAG v3.6.0
)

FetchContent_MakeAvailable(simdjson)

# System dependencies
find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED COMPONENTS system thread)
find_package(Threads REQUIRED)

# Core library
add_library(TradeAuraGate
    src/api.cpp
    src/websocket.cpp
    src/oms/RiskManager.cpp
    src/oms/OrderManager.cpp
    src/engine/RealtimeEngine.cpp
    src/engine/OptionSymbolManager.cpp
)

target_include_directories(TradeAuraGate
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(TradeAuraGate
    PUBLIC
        cpr::cpr
        simdjson::simdjson
        Boost::system
        Boost::thread
        OpenSSL::SSL
        OpenSSL::Crypto
        Threads::Threads
)

# Release optimization for GCC and Clang
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(TradeAuraGate PRIVATE
        $<$<CONFIG:Release>:-O3>
        $<$<CONFIG:Release>:-march=x86-64-v3>
        $<$<CONFIG:Release>:-mno-avx512f>
    )
endif()

# Trading engine
add_executable(nexus_engine examples/main.cpp)
target_link_libraries(nexus_engine PRIVATE TradeAuraGate)

# Tests
enable_testing()

add_executable(test_engine tests/test_engine.cpp)
target_link_libraries(test_engine PRIVATE TradeAuraGate)
add_test(NAME engine_tests COMMAND test_engine)

add_executable(test_oms tests/test_oms.cpp)
target_link_libraries(test_oms PRIVATE TradeAuraGate)
add_test(NAME oms_risk_tests COMMAND test_oms)

# Benchmarks
add_executable(benchmark_engine tests/benchmark_engine.cpp)
target_link_libraries(benchmark_engine PRIVATE TradeAuraGate)

add_executable(benchmark_real_ws tests/benchmark_real_ws.cpp)
target_link_libraries(benchmark_real_ws PRIVATE TradeAuraGate)

# Token test
add_executable(test_token tests/test_token.cpp)
target_link_libraries(test_token PRIVATE TradeAuraGate)
