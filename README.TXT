🧩 cpppm — C++ Package Manager (to become…)

to become the simplest, cross-platform, zero-dependency package manager for C++.

------------------------------------------------------------
🚀 Overview
------------------------------------------------------------
cpppm is a minimal package manager that treats dependencies as git + cmake glue.

No database, no registry —
just plain git clone + add_subdirectory() + CMake integration.

Ideal for small, experimental, or bootstrap projects
that want full reproducibility without an external service.

Directory layout example:

your_project/
 ├── CMakeLists.txt
 ├── cpppm.cfg
 ├── cmake/cpppm.cmake
 ├── external/
 └── tools/cpppm.cpp

------------------------------------------------------------
✨ Features
------------------------------------------------------------
- Zero installation — just compile tools/cpppm.cpp
- Simple manifest (cpppm.cfg)
- Git-based dependency fetch
- Auto-generated CMake stub
- Works on Linux / macOS / Windows
- Perfect for CMake + Git projects and embedded workflows

------------------------------------------------------------
🔧 Quick Start
------------------------------------------------------------
c++ -std=c++17 -O2 -o cpppm tools/cpppm.cpp
./cpppm init
./cpppm add fmt https://github.com/fmtlib/fmt.git tag=10.2.1
./cpppm add catch2 https://github.com/catchorg/Catch2.git tag=v3.5.4
./cpppm install
cmake -B build -S .
cmake --build build

------------------------------------------------------------
🧾 Example Manifest
------------------------------------------------------------
name=myproj
dep=fmt|https://github.com/fmtlib/fmt.git|tag=10.2.1|subdir=
dep=catch2|https://github.com/catchorg/Catch2.git|tag=v3.5.4|subdir=

Each line = dep=<name>|<git-url>|tag=<tag-or-branch>|subdir=<optional-path>

------------------------------------------------------------
⚙️ CMake Integration
------------------------------------------------------------
cmake_minimum_required(VERSION 3.16)
project(myproj CXX)
set(CMAKE_CXX_STANDARD 17)

add_executable(myproj src/main.cpp)
include(cmake/cpppm.cmake OPTIONAL)

------------------------------------------------------------
🧹 Clean Up
------------------------------------------------------------
./cpppm clean

Removes external/ and the generated CMake stub.

------------------------------------------------------------
🧠 Philosophy
------------------------------------------------------------
cpppm exists for clarity and reproducibility —
it’s not meant to compete with vcpkg or conan.
It’s for those who want a transparent, self-contained build
and the freedom to see every line of code you depend on.

No registry. No lock-in. Just Git and CMake.

------------------------------------------------------------
🔭 To be continued…
------------------------------------------------------------
Planned features:
- Dependency lockfile (sha1 pinning)
- Cached fetch for offline builds
- Parallel clone & update
- Integration with find_package
- cpppm publish (share manifest)

cpppm is still evolving — to become the simplest and most reliable
C++ package manager for small teams and solo devs.
