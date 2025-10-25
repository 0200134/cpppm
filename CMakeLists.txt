cmake_minimum_required(VERSION 3.16)
project(cpppm VERSION 0.1.0 LANGUAGES CXX)

# ==========================================================
# ⚙️ Build Settings
# ==========================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 기본 빌드 타입 지정 (Release)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# ==========================================================
# 🧩 Main Target
# ==========================================================
add_executable(cpppm
    src/main.cpp
)

target_include_directories(cpppm PRIVATE ${CMAKE_SOURCE_DIR}/include)

# ==========================================================
# 🧪 Tests
# ==========================================================
enable_testing()

if(EXISTS "${CMAKE_SOURCE_DIR}/tests/test_basic.cpp")
    add_executable(cpppm_tests tests/test_basic.cpp)
    add_test(NAME cpppm_smoke COMMAND cpppm_tests)
else()
    message(WARNING "⚠️ tests/test_basic.cpp not found, skipping cpppm_tests target.")
endif()
