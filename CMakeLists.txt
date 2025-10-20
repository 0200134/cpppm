cmake_minimum_required(VERSION 3.16)
project(cpppm VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(cpppm
    src/main.cpp
)

target_include_directories(cpppm PRIVATE ${CMAKE_SOURCE_DIR}/include)

# 테스트
enable_testing()
add_executable(cpppm_tests tests/test_basic.cpp)
add_test(NAME cpppm_smoke COMMAND cpppm_tests)
