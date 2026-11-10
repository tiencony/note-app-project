#ifndef DEBUG_H
#define DEBUG_H

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <system_error>
#include <vector>

// DLOG chi in khi bien dich voi TRACE=1 (make test-trace)
// Cach dung: DLOG("note=" << title << " size=" << content.size());
#ifdef NOTES_DEBUG
    #define DLOG(msg) \
        do { std::cerr << "[DBG " << __FILE__ << ":" << __LINE__ << "] " << msg << std::endl; } while(0)
#else
    #define DLOG(msg) do {} while(0)
#endif

// Cat ngan va hien thi ky tu xuong dong, de nhin noi dung tren mot dong
inline std::string preview(const std::string& s, std::size_t maxLen = 40){
    std::string out;
    for(char c : s){
        if(out.size() >= maxLen){ out += "..."; break; }
        if(c == '\n')      out += "\\n";
        else if(c == '\r') out += "\\r";
        else               out += c;
    }
    return out;
}

// In cac file .md tren dia (ten, so byte, noi dung dau). prefix de loc theo ten file
inline void dumpDisk(const std::string& dir = "vault/notes", const std::string& prefix = ""){
    namespace fs = std::filesystem;
    std::error_code ec;
    std::vector<fs::path> files;

    for(const auto& e : fs::directory_iterator(dir, ec)){
        if(!e.is_regular_file()) continue;
        if(e.path().filename().string().rfind(prefix, 0) != 0) continue;
        files.push_back(e.path());
    }
    std::sort(files.begin(), files.end());

    std::cout << "  [DISK] " << dir << "\n";
    if(files.empty()) std::cout << "    (trong)\n";
    for(const auto& p : files){
        std::ifstream in(p, std::ios::binary);
        std::stringstream ss;
        ss << in.rdbuf();
        const std::string content = ss.str();
        std::cout << "    - " << p.filename().string()
                  << " (" << content.size() << " byte): " << preview(content) << "\n";
    }
    std::cout << std::flush;
}

#endif