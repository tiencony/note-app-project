#include "../headers/NoteManager.h"
#include "../headers/Utils.h"
#include "../headers/Note.h"

#include <fstream>
#include <filesystem>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <set>
#include <map>

namespace fs = std::filesystem;
using Tags = std::set<std::string>;



std::vector<std::string> NoteManager::brokenLinks(const std::string& title) const{
    std::vector<std::string> links;
    for(const auto& link : )
}