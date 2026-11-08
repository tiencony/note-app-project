// #include "../headers/NoteManager.h"
// #include "../headers/Utils.h"
// #include "../headers/Note.h"

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


std::set<std::string> extractLinks(const std::string& content){
    std::set<std::string> links;
    std::size_t pos = 0;

    while(true){
        std::size_t open = content.find("[[", pos);
        if(open == std::string::npos) break;

        std::size_t close = content.find("]]", open + 2);
        if(close == std::string::npos) break;

        std::string link = content.substr(open + 2, close - (open + 2));

        if(link.find_first_of("\n[") != std::string::npos){
            pos = open + 1;
            continue;
        }
        pos = close + 2;
        link = trimSpaces(link);
        if(link.empty()) continue;
        links.insert(link);
    }
    return links;
}

int main(){
    std::set<std::string> extractedLinks = extractLinks("[[zz_test_L2]] va [[zz_test_khong_co]]");
    for(const auto& link : extractedLinks){
        std::cout << link << std::endl;
    }
    return 0;
}