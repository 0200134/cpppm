// cpppm 0.4-pre
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <filesystem>
#include <future>
#include <mutex>
#include <cstdlib>
#include <sstream>
#include <vector>
#include "cpppm/version.h"

namespace fs = std::filesystem;
std::mutex cout_mtx;
std::mutex lock_mtx;

struct Manifest { std::map<std::string, std::string> deps; };
static fs::path manifestPath() { return fs::current_path() / "cpppm.toml"; }
static fs::path depsDir() { return fs::current_path() / "deps"; }
static fs::path lockPath() { return fs::current_path() / "cpppm.lock"; }
static fs::path cmakeDepsPath() { return depsDir() / "CMakeDeps.cmake"; }

static std::string trim(const std::string &s) {
    size_t b = 0, e = s.size();
    while (b < e && isspace((unsigned char)s[b])) ++b;
    while (e > b && isspace((unsigned char)s[e - 1])) --e;
    return s.substr(b, e - b);
}

static Manifest readManifest() {
    Manifest m;
    std::ifstream in(manifestPath());
    if (!in.is_open()) return m;
    std::string line; bool inDeps = false;
    while (std::getline(in, line)) {
        line = trim(line);
        if (line == "[dependencies]") { inDeps = true; continue; }
        if (!inDeps || line.empty() || line[0] == '#') continue;
        auto pos = line.find('=');
        if (pos == std::string::npos) continue;
        std::string name = trim(line.substr(0, pos));
        std::string url = trim(line.substr(pos + 1));
        if (!url.empty() && url.front() == '"' && url.back() == '"')
            url = url.substr(1, url.size() - 2);
        m.deps[name] = url;
    }
    return m;
}

static void system_checked(const std::string &cmd) {
    int ret = std::system(cmd.c_str());
    if (ret != 0) throw std::runtime_error("Command failed: " + cmd);
}

static std::string git_head_commit(const fs::path &dir) {
    std::string cmd = "git -C \"" + dir.string() + "\" rev-parse HEAD";
    FILE *pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "unknown";
    char buf[128];
    std::string hash;
    while (fgets(buf, sizeof(buf), pipe)) hash += buf;
    pclose(pipe);
    if (!hash.empty() && hash.back() == '\n') hash.pop_back();
    return hash;
}

static void install_one(const std::string &name, const std::string &url,
                        std::map<std::string, std::string> &lock) {
    fs::path pkgDir = depsDir() / name;
    {
        std::lock_guard<std::mutex> lk(cout_mtx);
        std::cout << "[cpppm] Installing " << name << "...\n";
    }
    if (fs::exists(pkgDir)) {
        std::lock_guard<std::mutex> lk(cout_mtx);
        std::cout << "  -> " << name << " already exists. Skipping.\n";
        std::lock_guard<std::mutex> lk2(lock_mtx);
        lock[name] = git_head_commit(pkgDir);
        return;
    }

    fs::create_directories(depsDir());
    std::string cloneCmd = "git clone --depth 1 " + url + " \"" + pkgDir.string() + "\"";
    try {
        system_checked(cloneCmd);
    } catch (...) {
        std::lock_guard<std::mutex> lk(cout_mtx);
        std::cerr << "  -> clone failed for " << name << ", retrying...\n";
        system_checked(cloneCmd);
    }

    fs::path cmakeFile = pkgDir / "CMakeLists.txt";
    if (fs::exists(cmakeFile)) {
        fs::path buildDir = pkgDir / "build";
        fs::create_directories(buildDir);
        try {
            system_checked("cmake -B \"" + buildDir.string() + "\" -S \"" + pkgDir.string() + "\"");
            system_checked("cmake --build \"" + buildDir.string() + "\" --parallel");
        } catch (...) {
            std::lock_guard<std::mutex> lk(cout_mtx);
            std::cerr << "  -> build failed for " << name << "\n";
        }
    }

    {
        std::lock_guard<std::mutex> lk(lock_mtx);
        lock[name] = git_head_commit(pkgDir);
    }
    std::ofstream ok(pkgDir / "INSTALL_OK");
    ok << name << " built by cpppm " << CPPPM_VERSION << "\n";
}

static void cmd_install() {
    Manifest m = readManifest();
    if (m.deps.empty()) {
        std::cout << "cpppm: no dependencies found.\n"; return;
    }
    fs::create_directories(depsDir());

    std::map<std::string, std::string> lock;
    std::vector<std::future<void>> tasks;
    unsigned threads = std::thread::hardware_concurrency();
    if (threads < 2) threads = 2;
    std::cout << "[cpppm] Using " << threads << " threads.\n";

    for (auto &kv : m.deps) {
        tasks.push_back(std::async(std::launch::async, [&] {
            try { install_one(kv.first, kv.second, lock); }
            catch (const std::exception &e) {
                std::lock_guard<std::mutex> lk(cout_mtx);
                std::cerr << "[error] " << kv.first << ": " << e.what() << "\n";
            }
        }));
    }

    for (auto &t : tasks) t.wait();

    std::ofstream out(lockPath());
    out << "# cpppm lockfile\n";
    for (auto &kv : lock)
        out << kv.first << " = \"" << kv.second << "\"\n";

    std::ofstream cmakeDeps(cmakeDepsPath());
    for (auto &kv : m.deps)
        cmakeDeps << "add_subdirectory(${CMAKE_SOURCE_DIR}/deps/" << kv.first << ")\n";

    std::cout << "[cpppm] lockfile written: cpppm.lock\n";
    std::cout << "[cpppm] generated: deps/CMakeDeps.cmake\n";
}

enum class Cmd { Install, Help, Unknown };
static Cmd parse_cmd(const std::string &s) {
    if (s == "install") return Cmd::Install;
    if (s == "help") return Cmd::Help;
    return Cmd::Unknown;
}

static void help() {
    std::cout << "cpppm " << CPPPM_VERSION << "\n"
              << "Usage:\n"
              << "  cpppm install   Install dependencies\n"
              << "  cpppm help      Show this message\n";
}

int main(int argc, char **argv) {
    if (argc < 2) { help(); return 0; }
    Cmd cmd = parse_cmd(argv[1]);
    switch (cmd) {
        case Cmd::Install: cmd_install(); break;
        case Cmd::Help: help(); break;
        default: help(); break;
    }
    return 0;
}
