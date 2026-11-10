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