#include "Utils/Parser.h"
#include "Utils/StringUtils.h"   // extractLinks goi trimSpaces

#include <cctype>
#include <algorithm>
#include <vector>
#include <string>


namespace {
    bool canPrecedeTag(char c){
        return std::isspace(static_cast<unsigned char>(c)) || c == '(' || c == '"';
    }
}


std::set<std::string> extractTags(const std::string& content){
    std::set<std::string> tags;
    std::vector<LinkSpan> links = findLinkSpans(content);

    auto isInLink = [&](std::size_t pos){
        for(const auto& link : links){
            // Vùng link tính từ dấu [ đầu tiên đến dấu ] cuối cùng 
            if(pos >= (link.innerBegin - 2) && pos <= (link.innerEnd + 2)){
                return true;
            }
        }     
        return false;
    };
    const std::size_t len = content.size();
    for(std::size_t i = 0; i < len; i++){
        if(content[i] != '#') continue;
        if(i != 0 && !canPrecedeTag(content[i - 1])) continue;
        if(isInLink(i)) continue;

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

LinkParts parseLink(const std::string& inner){
    LinkParts result;
    std::size_t dividePos = inner.find_first_of("|#");

    if(dividePos == std::string::npos){
        result.target = trimSpaces(inner);
        result.suffix = "";
    }else{
        result.target = trimSpaces(inner.substr(0, dividePos));
        result.suffix = inner.substr(dividePos);
    }
    return result;
}

std::vector<LinkSpan> findLinkSpans(const std::string& content){
    std::vector<LinkSpan> result;
    std::size_t pos = 0;
    
    while(true){
        std::size_t open = content.find("[[", pos);
        if(open == std::string::npos) break;
        std::size_t close = content.find("]]", open + 2);
        if(close == std::string::npos) break;
        
        std::string_view inner(content.data() + open + 2, close - (open + 2));

        if(inner.find_first_of("\n[") != std::string_view::npos){
            pos = open + 1;
            continue;
        }
        result.push_back({open + 2, close});
        pos = close + 2;

    }
   return result;
}

std::set<std::string> extractLinks(const std::string& content){
    std::set<std::string> links;
    for(const LinkSpan& s : findLinkSpans(content)){
        std::string link = trimSpaces(parseLink(content.substr(s.innerBegin, s.innerEnd - s.innerBegin)).target);
        if(!link.empty()){
            links.insert(link);
        }
    }
    return links;
}



// Chi doi tren RAM 
std::string replaceLink(const std::string& content, const std::string& oldTitle, const std::string& newTitle){
    const std::string oldkey = normalizeTitle(oldTitle);
    if(oldkey.empty()) return content;

    std::string results;
    std::size_t lastCopiedPos = 0;

    
    for(const LinkSpan& s : findLinkSpans(content)){
        std::string inner = content.substr(s.innerBegin, s.innerEnd - s.innerBegin);
        
        LinkParts parts = parseLink(inner); // tach target va suffix

        if(normalizeTitle(parts.target) != oldkey) continue;
        
        // Noi tu moc cu den truoc dau [[
        results.append(content, lastCopiedPos, s.innerBegin - 2 - lastCopiedPos); // Tru 2 cho [[
        results += "[[" + newTitle + parts.suffix + "]]";
        lastCopiedPos = s.innerEnd + 2; // Cap nhat khi da quet xong sau ]]
    }
    // Noi phan duoi con lai
    results.append(content, lastCopiedPos, std::string::npos);
    return results;
}




