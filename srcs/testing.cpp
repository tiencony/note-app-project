#include "Utils/Parser.h"
#include "Utils/StringUtils.h"   // extractLinks goi trimSpaces

#include <cctype>
#include <vector>
#include <string>
#include <string_view>
#include <filesystem>
#include <fstream>
#include <system_error>

#ifdef _WIN32
  #ifndef NOMINMAX
    #define NOMINMAX
  #endif
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
#endif



namespace fs = std::filesystem;


std::string normalizeContent(std::string_view s){
    // cat bo BOM-UTF8
    if(s.size() >= 3 &&
        static_cast<unsigned char>(s[0]) == 0xEF &&
        static_cast<unsigned char>(s[1]) == 0xBB &&
        static_cast<unsigned char>(s[2]) == 0xBF){
            s.remove_prefix(3);
        }
    
    std::string out;
    out.reserve(s.size());
    for(std::size_t i = 0; i < s.size(); i++){
        if(s[i] == '\r'){
            out.push_back('\n');
            if(i + 1 < s.size() && s[i + 1] == '\n') i++; // "\r\n" chi thanh 1 '\n'
        }else{
            out.push_back(s[i]);
        }
    }
    return out;   
}


bool readFileToString(const fs::path& path, std::string& out){
    std::ifstream inFile(path, std::ios::binary);
    if(!inFile) return false;

    inFile.seekg(0, std::ios::end);
    const std::streamoff size = inFile.tellg();
    if(size < 0) return false;
    inFile.seekg(0, std::ios::beg);

    std::string s(static_cast<std::size_t>(size), '\0');
    inFile.read(s.data(), size);
    if(inFile.gcount() != size) return false;
    out = normalizeContent(s);
    return true;
}


#ifdef _WIN32

bool writeFileAtomic(const fs::path& path, const std::string& content, bool overwrite){
    fs::path tmp = path;
    tmp += L".tmp";

    HANDLE h = CreateFileW(tmp.c_str(), GENERIC_WRITE, 0, nullptr,
                           CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if(h == INVALID_HANDLE_VALUE) return false;

    bool ok = true;
    const char* p = content.data();
    std::size_t left = content.size();
    while(left > 0){
        const DWORD chunk = static_cast<DWORD>(std::min<std::size_t>(left, 1u << 30));
        DWORD written = 0;
        if(!WriteFile(h, p, chunk, &written, nullptr) || written == 0){ ok = false; break; }
        p += written;
        left -= written;
    }
    if(ok && !FlushFileBuffers(h)) ok = false;    // <- "fsync" của Windows
    CloseHandle(h);

    if(ok){
        DWORD flags = MOVEFILE_WRITE_THROUGH;
        if(overwrite) flags |= MOVEFILE_REPLACE_EXISTING;   // không có cờ này: đích tồn tại -> lỗi
        ok = MoveFileExW(tmp.c_str(), path.c_str(), flags) != 0;
    }
    if(!ok){
        std::error_code ec;
        fs::remove(tmp, ec);
    }
    return ok;
}

#else   // ban du phong de build duoc tren os khac (chua fsync)

bool writeFileAtomic(const fs::path& path, const std::string& content, bool overwrite){
    std::error_code ec;
    if(!overwrite && fs::exists(path, ec)) return false;

    fs::path tmp = path;
    tmp += ".tmp";
    {
        std::ofstream out(tmp, std::ios::binary | std::ios::trunc);
        if(!out) return false;
        out.write(content.data(), static_cast<std::streamsize>(content.size()));
        out.close();
        if(out.fail()){ fs::remove(tmp, ec); return false; }
    }
    fs::rename(tmp, path, ec);
    if(ec){ fs::remove(tmp, ec); return false; }
    return true;
}

#endif