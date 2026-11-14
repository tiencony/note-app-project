#include "Utils/Parser.h"
#include "Utils/StringUtils.h"   // extractLinks goi trimSpaces

#include <cctype>
#include <vector>
#include <string>
#include <string_view>

namespace {
    bool canPrecedeTag(char c){
        return std::isspace(static_cast<unsigned char>(c)) || c == '(' || c == '"';
    }
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


struct LinkParts{
    std::string target; // Da cat dau "|, #"
    std::string suffix; // Sau dau "|, #"
};

