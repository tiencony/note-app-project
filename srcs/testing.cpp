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
#include <iostream>
#include <vector>

std::string trimSpaces(const std::string& s){
    std::size_t begin = s.find_first_not_of(' ');
    if(begin == std::string::npos) return "";
    std::size_t end = s.find_last_not_of(' ');
    return s.substr(begin, end - begin + 1);
}



std::vector<BrokenLink> NoteManager::brokenLinks() const{
    std::vector<BrokenLink> results;
    for(const auto& [sourceKey, links] : linksBySource){
        auto sourceTitle = titleByKey.find(sourceKey);
        if(sourceTitle == titleByKey.end()) continue;

        for(const std::string& link : links){
            if(titleByKey.find(normalizeTitle(link)) == titleByKey.end()){
                results.push_back({sourceTitle->second, link});
            }
        }
    }

}


