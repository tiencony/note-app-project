#include "Utils/FileUtils.h"

#include <fstream>
#include <sstream>
#include <algorithm>

namespace fs = std::filesystem;

bool readFileToString(const fs::path& path, std::string& out){

    std::ifstream inFile(path);
    if(!inFile) return false;
    std::stringstream buffer;
    buffer << inFile.rdbuf();


    std::string s = buffer.str();

    if(s.size() >= 3 && static_cast<unsigned char>(s[0]) == 0xEF &&
    static_cast<unsigned char>(s[1]) == 0xBB && static_cast<unsigned char>(s[2]) == 0xBF){
        s.erase(0, 3);                                            // bo BOM
    }
    s.erase(std::remove(s.begin(), s.end(), '\r'), s.end());      // CRLF -> LF (can <algorithm>)

    out = std::move(s);
    return true;
}



bool writeFileAtomic(const fs::path& path, const std::string& content){
    fs::path tmp = path;
    tmp += ".tmp";
    std::error_code ec;
    {
        std::ofstream out(tmp);
        if(!out) return false;
        out << content;
        out.close();
        if(out.fail()){
            fs::remove(tmp, ec);
            return false;
        }
    }
    fs::rename(tmp, path, ec);
    if(ec){
        fs::remove(tmp, ec);
        return false;
    }
    return true;

}

