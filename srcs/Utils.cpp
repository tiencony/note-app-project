#include "../headers/Utils.h"
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <cctype>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <set>


namespace fs = std::filesystem;

// Tham khao phan Time
std::string formatTime(std::time_t t){
    std::tm* local = std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(local, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string getCurrentTime(){
    return formatTime(std::time(nullptr));
}

std::string getFileTime(const fs::path& p){
    std::error_code ec;
    auto ftime = fs::last_write_time(p, ec);
    if(ec) return "Unknown";

    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
    return formatTime(std::chrono::system_clock::to_time_t(sctp));
}


bool equalsIgnoreCase(const std::string& str1, const std::string& str2){
    if(str1.length() != str2.length()) return false;
    
    return std::equal(
        str1.begin(), str1.end(),
        str2.begin(), str2.end(),
        [](unsigned char ch1, unsigned char ch2){
            return std::tolower(ch1) == std::tolower(ch2);
        }
    );
    
}


std::size_t findIgnoreCase(const std::string& text, const std::string& keyword){
    auto it = std::search(
        text.begin(), text.end(),
        keyword.begin(), keyword.end(),
        [](unsigned char a, unsigned char b){
            return std::tolower(a) == std::tolower(b);
        }
    );
    if(it == text.end()) return std::string::npos;
    return it - text.begin();
}


bool containsIgnoreCase(const std::string& text, const std::string& keyword){
    return findIgnoreCase(text, keyword) != std::string::npos;
}



std::string makeSnippet(const std::string& text, std::size_t pos, std::size_t keywordLen){
    const std::size_t margin = 20;
    std::size_t start = (pos > margin) ? pos - margin : 0;
    std::size_t end = std::min(text.size(), pos + keywordLen + margin);

    std::string snippet = text.substr(start, end - start);
    std::replace(snippet.begin(), snippet.end(), '\n', ' ');

    std::size_t keywordPosInSnippet = (pos > margin) ? margin : pos;

    snippet.insert(keywordPosInSnippet + keywordLen, "\"");
    snippet.insert(keywordPosInSnippet, "\"");

    if (start > 0) snippet = "..." + snippet;
    if (end < text.size()) snippet += "...";

    return snippet;
}


bool readFileToString(const fs::path& path, std::string& out){
    std::ifstream inFile(path);
    if(!inFile) return false;
    std::stringstream buffer;
    buffer << inFile.rdbuf();
    out = buffer.str();
    return true;
}



static bool canPrecedeTag(char c){
    return std::isspace(static_cast<unsigned char>(c)) || c == '(' || c == '"';
}


std::set<std::string> extractTags(const std::string& content){
    std::set<std::string> tags;
    const std::size_t len = content.size();

    for(std::size_t i = 0; i < len; i++){
        if(content[i] != '#') continue;

        
        if(i != 0 && !canPrecedeTag(content[i - 1])){
            continue;
        }

        // check sau # phai la chu hoac so
        std::size_t j = i + 1;
        if(j >= len || !std::isalnum(static_cast<unsigned char>(content[j]))){
            continue;
        }

        std::string tag;
        while(j < len){
            unsigned char ch = content[j];
            if(std::isalnum(ch) || ch == '_' || ch == '-'){
                tag += static_cast<char>(std::tolower(ch));
                j++;
            }else break;
        }
        tags.insert(tag);
        i = j - 1;
    }
    return tags;
}




std::string normalizeTag(const std::string& tag){
    if(tag.empty()) return "";

    auto it = tag.begin();
    if(*it == '#') it++;
    std::string resTag = "";
    for(; it != tag.end(); it++){
        resTag += static_cast<char>(std::tolower(static_cast<unsigned char>(*it)));
    }

    return resTag;
}


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


