#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <filesystem>
#include <cstdlib>
#include <sstream>
#include "cpppm/version.h"

namespace fs = std::filesystem;

struct Manifest {
    std::map<std::string, std::string> deps;
};

static fs::path manifestPath() { return fs::current_path() / "cpppm.toml"; }
static fs::path depsDir() { return fs::current_path() / "deps"; }

static std::string trim(const std::string &s){
    size_t b=0, e=s.size();
    while(b<e && isspace((unsigned char)s[b])) ++b;
    while(e>b && isspace((unsigned char)s[e-1])) --e;
    return s.substr(b,e-b);
}

static Manifest readManifest(){
    Manifest m;
    std::ifstream in(manifestPath());
    if(!in.is_open()) return m;
    std::string line; bool inDeps=false;
    while(std::getline(in,line)){
        line=trim(line);
        if(line=="[dependencies]") { inDeps=true; continue; }
        if(!inDeps || line.empty() || line[0]=='#') continue;
        auto pos=line.find('=');
        if(pos==std::string::npos) continue;
        std::string name=trim(line.substr(0,pos));
        std::string url=trim(line.substr(pos+1));
        if(!url.empty() && url.front()=='"' && url.back()=='"')
            url=url.substr(1,url.size()-2);
        m.deps[name]=url;
    }
    return m;
}

static void system_checked(const std::string &cmd){
    std::cout << "  -> $ " << cmd << std::endl;
    int ret = std::system(cmd.c_str());
    if(ret != 0) throw std::runtime_error("Command failed: "+cmd);
}

static void install_one(const std::string &name,const std::string &url){
    fs::path pkgDir=depsDir()/name;
    if(fs::exists(pkgDir)) {
        std::cout << name << " already installed, skipping.\n";
        return;
    }
    std::cout << "Cloning "<<name<<" from "<<url<<"...\n";
    fs::create_directories(depsDir());
    system_checked("git clone --depth 1 "+url+" "+pkgDir.string());

    fs::path cmakeFile = pkgDir/"CMakeLists.txt";
    if(fs::exists(cmakeFile)){
        std::cout << "Building "<<name<<" via CMake...\n";
        fs::path buildDir = pkgDir/"build";
        fs::create_directories(buildDir);
        system_checked("cmake -B "+buildDir.string()+" -S "+pkgDir.string());
        system_checked("cmake --build "+buildDir.string()+" --parallel 4");
    }
    std::ofstream ok(pkgDir/"INSTALL_OK");
    ok << name << " built by cpppm "<<CPPPM_VERSION<<"\n";
}

static void cmd_install(){
    Manifest m=readManifest();
    if(m.deps.empty()){
        std::cout << "cpppm: no deps.\n"; return;
    }
    fs::create_directories(depsDir());
    for(auto &kv:m.deps){
        try { install_one(kv.first,kv.second); }
        catch(const std::exception &e){
            std::cerr << "Error installing "<<kv.first<<": "<<e.what()<<"\n";
        }
    }
    std::cout << "cpppm: install complete.\n";
}

static void help(){
    std::cout << "cpppm "<<CPPPM_VERSION<<"\n"
              << "Usage: cpppm <command> [args]\n"
              << "Commands:\n"
              << "  install   clone + build deps via cmake\n"
              << "  help      show this help\n";
}

int main(int argc,char**argv){
    if(argc<2){ help(); return 0; }
    std::string cmd=argv[1];
    if(cmd=="install") cmd_install();
    else help();
    return 0;
}
