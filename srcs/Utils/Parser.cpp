#include "Utils/Parser.h"
#include "Utils/StringUtils.h"   // extractLinks goi trimSpaces

#include <cctype>
#include <algorithm>

namespace {
    bool canPrecedeTag(char c){
        return std::isspace(static_cast<unsigned char>(c)) || c == '(' || c == '"';
    }
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
    std::string t = trimSpaces(tag);
    if(!t.empty() && t[0] == '#') t.erase(0, 1);
    std::transform(t.begin(), t.end(), t.begin(),
        [](unsigned char ch){ return static_cast<char>(std::tolower(ch)); });
    return t;
}


std::string normalizeTitle(const std::string& s){
    std::string out = trimSpaces(s);
    std::transform(out.begin(), out.end(), out.begin(),
        [](unsigned char ch){ return static_cast<char>(std::tolower(ch)); });
    if(out.size() > 3 && out.compare(out.size() - 3, 3, ".md") == 0){
        out.erase(out.size() - 3);
    }
    return out;
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



// Chi doi tren RAM 
std::string replaceLink(const std::string& content, const std::string& oldTitle, const std::string& newTitle){
    const std::string oldkey = normalizeTitle(oldTitle);
    if(oldkey.empty()) return content;

    std::string results;
    std::size_t copied = 0;
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
        if(normalizeTitle(link) == oldkey){
            results.append(content, copied, open + 2 - copied);
            results += newTitle;
            copied = close;
        }
    }
    results.append(content, copied, std::string::npos);
    return results;
}


