#include "Utils/FileUtils.h"

#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

bool readFileToString(const fs::path& path, std::string& out){
    std::ifstream inFile(path);
    if(!inFile) return false;
    std::stringstream buffer;
    buffer << inFile.rdbuf();
    out = buffer.str();
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

