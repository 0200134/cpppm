#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <filesystem>
#include <optional>
#include <sstream>
#include "cpppm/version.h"

namespace fs = std::filesystem;

struct Manifest {
    std::string name = "";
    std::string version = "0.0.0";
    std::map<std::string, std::string> deps; // name -> url
};

static fs::path manifestPath() { return fs::current_path() / "cpppm.toml"; }
static fs::path depsDir()      { return fs::current_path() / "deps"; }

static std::string trim(const std::string &s){
    size_t b=0, e=s.size();
    while(b<e && isspace(static_cast<unsigned char>(s[b]))) ++b;
    while(e>b && isspace(static_cast<unsigned char>(s[e-1]))) --e;
    return s.substr(b, e-b);
}

static Manifest readManifest(){
    Manifest m;
    std::ifstream in(manifestPath());
    if(!in.is_open()) return m; // empty manifest

    std::string line; bool inDeps=false;
    while(std::getline(in, line)){
        line = trim(line);
        if(line.empty() || line[0]=='#') continue;
        if(line.rfind("name",0)==0){
            auto pos = line.find('=');
            if(pos!=std::string::npos){ m.name = trim(line.substr(pos+1));
                if(!m.name.empty() && m.name.front()=='"' && m.name.back()=='"')
                    m.name = m.name.substr(1, m.name.size()-2);
            }
        } else if(line.rfind("version",0)==0){
            auto pos=line.find('=');
            if(pos!=std::string::npos){ m.version = trim(line.substr(pos+1));
                if(!m.version.empty() && m.version.front()=='"' && m.version.back()=='"')
                    m.version = m.version.substr(1, m.version.size()-2);
            }
        } else if(line=="[dependencies]"){
            inDeps=true; continue;
        } else if(inDeps){
            // key = "value"
            auto pos=line.find('=');
            if(pos!=std::string::npos){
                std::string key = trim(line.substr(0,pos));
                std::string val = trim(line.substr(pos+1));
                if(!val.empty() && val.front()=='"' && val.back()=='"')
                    val = val.substr(1, val.size()-2);
                if(!key.empty()) m.deps[key]=val;
            }
        }
    }
    return m;
}

static void writeManifest(const Manifest &m){
    std::ofstream out(manifestPath());
    out << "name = \"" << (m.name.empty()?"cpppm_project":m.name) << "\"\n";
    out << "version = \"" << (m.version.empty()?"0.1.0":m.version) << "\"\n\n";
    out << "[dependencies]\n";
    for(const auto &kv : m.deps){
        out << kv.first << " = \"" << kv.second << "\"\n";
    }
}

static void cmd_init(){
    if(fs::exists(manifestPath())){
        std::cout << "cpppm: cpppm.toml already exists.\n";
        return;
    }
    Manifest m; m.name = fs::current_path().filename().string(); m.version = "0.1.0";
    writeManifest(m);
    std::cout << "cpppm: initialized cpppm.toml\n";
}

static void cmd_add(const std::string &name, const std::string &url){
    Manifest m = readManifest();
    if(m.name.empty()) m.name = fs::current_path().filename().string();
    if(m.version.empty()) m.version = "0.1.0";
    m.deps[name]=url;
    writeManifest(m);
    std::cout << "cpppm: added dependency '"<<name<<"' -> "<<url<<"\n";
}

static void fake_install_one(const std::string &name){
    fs::create_directories(depsDir()/name);
    std::ofstream ok(depsDir()/name/"INSTALL_OK");
    ok << name << " installed by cpppm " << CPPPM_VERSION << "\n";
}

static void cmd_install(){
    Manifest m = readManifest();
    if(m.deps.empty()){
        std::cout << "cpppm: no dependencies.\n"; return;
    }
    fs::create_directories(depsDir());
    for(const auto &kv : m.deps){
        std::cout << "Installing "<<kv.first<<" from "<<kv.second<<" ...\n";
        fake_install_one(kv.first);
    }
    std::cout << "cpppm: install complete.\n";
}

static void cmd_list(){
    Manifest m = readManifest();
    std::cout << "Dependencies ("<<m.deps.size()<<"):\n";
    for(const auto &kv : m.deps){
        std::cout << " - "<<kv.first<<" : "<<kv.second<<"\n";
    }
}

static void cmd_sync(){
    Manifest m = readManifest();
    fs::create_directories(depsDir());
    for(const auto &kv : m.deps){
        if(!fs::exists(depsDir()/kv.first/"INSTALL_OK")){
            std::cout << "Sync: installing missing "<<kv.first<<"...\n";
            fake_install_one(kv.first);
        }
    }
    std::cout << "cpppm: sync complete.\n";
}

static void help(){
    std::cout << "cpppm " << CPPPM_VERSION << "\n"
              << "Usage: cpppm <command> [args]\n\n"
              << "Commands:\n"
              << "  init                Initialize cpppm.toml\n"
              << "  add <name> <url>    Add dependency\n"
              << "  install             Install deps to ./deps (fake)\n"
              << "  list                Show manifest deps\n"
              << "  sync                Ensure ./deps matches manifest\n"
              << "  help                Show this message\n";
}

int main(int argc, char** argv){
    if(argc < 2){ help(); return 0; }
    std::string cmd = argv[1];
    try{
        if(cmd=="init") cmd_init();
        else if(cmd=="add"){
            if(argc<4){ std::cerr << "cpppm: add <name> <url>\n"; return 1; }
            cmd_add(argv[2], argv[3]);
        }
        else if(cmd=="install") cmd_install();
        else if(cmd=="list") cmd_list();
        else if(cmd=="sync") cmd_sync();
        else if(cmd=="help" || cmd=="-h" || cmd=="--help") help();
        else { std::cerr << "cpppm: unknown command '"<<cmd<<"'\n"; return 1; }
    } catch(const std::exception &e){
        std::cerr << "cpppm: error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
