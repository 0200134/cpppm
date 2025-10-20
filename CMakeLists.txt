cmake_minimum_required(VERSION 3.20)
project(Cpppm VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include path
include_directories(include)

# Add executable
add_executable(cpppm
    src/main.cpp
)
