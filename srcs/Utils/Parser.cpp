#include "Utils/Parser.h"
#include "Utils/StringUtils.h"   // extractLinks goi trimSpaces

#include <cctype>
#include <algorithm>
#include <vector>
#include <string>
#include <sstream>
#include <string_view>
#include <optional>

namespace {
bool isTagChar(unsigned char ch){
    return std::isalnum(ch) || ch == '_' || ch == '-';
}

bool isValidTag(std::string_view tag){
    if(tag.empty() || !std::isalnum(static_cast<unsigned char>(tag.front()))) return false;
    for(char c : tag){
        if(!isTagChar(static_cast<unsigned char>(c))) return false;
    }
    return true;
}

bool canPrecedeTag(char c) {
    return std::isspace(static_cast<unsigned char>(c)) || c == '(' || c == '"';
}



// Ham tach bo dau ngoac kep: "..." hoac '...'.
std::string_view unquote(std::string_view s) {
    if (s.size() >= 2) {
        if ((s.front() == '"' && s.back() == '"') ||
            (s.front() == '\'' && s.back() == '\'')) {
            return s.substr(1, s.size() - 2);
        }
    }
    return s;
}

bool hasLineBreak(std::string_view s){
    return s.find_first_of("\r\n") != std::string_view::npos;
}

// Ham noi bo dung cho parseKeyValueLine
struct KeyValue{
    std::string_view key;
    std::string_view value;
};

// Tra ve cap key, value theo dong
std::optional<KeyValue> parseKeyValueLine(std::string_view line){
    if(line.empty()) return std::nullopt;
    const char first = line.front();
    if(first == ' ' || first == '\t' || first == '-' || first == '#') return std::nullopt;
    

    const std::size_t colonPos = line.find(':');
    if (colonPos == std::string_view::npos) return std::nullopt;

    const std::string_view key = trimSpaces(line.substr(0, colonPos));
    if(key.empty()) return std::nullopt;
    return KeyValue{key, unquote(trimSpaces(line.substr(colonPos + 1)))};
}


// true neu `line` la dong "khoa: ..." voi dung khoa can tim.
// Chi tra ve bool (khong tra view) de noi goi co the SUA `line` ngay sau do ma khong lo view treo.
bool isKeyLine(std::string_view line, std::string_view key){
    const auto kv = parseKeyValueLine(line);
    return kv && kv->key == key;
}

} // dong namespace





std::string normalizeTag(std::string_view tag){
    std::string_view t = trimSpaces(tag);
    if(!t.empty() && t.front() == '#'){
        t.remove_prefix(1);
    }

    std::string out(t);
    std::transform(out.begin(), out.end(), out.begin(),
                    [](unsigned char ch){ return static_cast<char>(std::tolower(ch)); });

    return out;
}



std::string normalizeTitle(std::string_view s){
    std::string out(trimSpaces(s));
    std::transform(out.begin(), out.end(), out.begin(),
        [](unsigned char ch){ return static_cast<char>(std::tolower(ch)); });
    if(out.size() > 3 && out.compare(out.size() - 3, 3, ".md") == 0){
        out.erase(out.size() - 3);
    }
    return out;
}





LinkParts parseLink(std::string_view inner){
    LinkParts result;
    const std::size_t dividePos = inner.find_first_of("|#");

    if(dividePos == std::string_view::npos){
        result.target = trimSpaces(inner);
        result.suffix = "";
    }else{
        result.target = trimSpaces(inner.substr(0, dividePos));
        result.suffix = inner.substr(dividePos);
    }
    return result;
}


std::vector<LinkSpan> findLinkSpans(std::string_view content){
    std::vector<LinkSpan> result;
    std::size_t pos = 0;
    
    while(true){
        std::size_t open = content.find("[[", pos);
        if(open == std::string_view::npos) break;
        std::size_t close = content.find("]]", open + 2);
        if(close == std::string_view::npos) break;
        
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

std::set<std::string> extractLinks(std::string_view content) {
    std::set<std::string> links;
    for(const LinkSpan& s : findLinkSpans(content)){
        const LinkParts parts = parseLink(content.substr(s.innerBegin, s.innerEnd - s.innerBegin));

        if (!parts.target.empty()) {
            links.emplace(std::string(parts.target));  // cap phat dung 1 lan khi luu vao set
        }
    }
    return links;
}



// Chi doi tren RAM 
std::string replaceLink(std::string_view content, std::string_view oldTitle, std::string_view newTitle){
    const std::string oldkey = normalizeTitle(oldTitle);
    if (oldkey.empty()) return std::string(content);

    std::string result;
    std::size_t lastCopiedPos = 0;

    
    for(const LinkSpan& s : findLinkSpans(content)){
        std::string_view inner = content.substr(s.innerBegin, s.innerEnd - s.innerBegin);
        
        LinkParts parts = parseLink(inner); // tach target va suffix

        if(normalizeTitle(parts.target) != oldkey) continue;
        
        result += content.substr(lastCopiedPos, (s.innerBegin - 2) - lastCopiedPos);   // den truoc "[["
        result += "[[";
        result += newTitle;
        result += parts.suffix;         
        

        result += "]]";
        lastCopiedPos = s.innerEnd + 2; // Cap nhat khi da quet xong sau ]]
    }
    // Noi phan duoi con lai
    result += content.substr(lastCopiedPos);
    return result;
}









// Chua sua xong
FrontmatterBlock splitFrontmatter(std::string_view content){
    FrontmatterBlock block;

    // Dong dau phai la "---"
    std::size_t firstNewLine = content.find('\n');
    if(firstNewLine == std::string_view::npos || content.substr(0, firstNewLine) != "---") return block;


    std::vector<std::string> lines;
    std::size_t currPos = firstNewLine + 1;

    while(currPos <= content.size()){
        std::size_t nextNewline = content.find('\n', currPos);

        
        const std::string_view line = (nextNewline == std::string_view::npos)
                                    ? content.substr(currPos) : content.substr(currPos, nextNewline - currPos);

        if(line == "---"){
            block.hasFrontmatter = true;
            block.lines = std::move(lines);
            block.bodyOffset = (nextNewline == std::string_view::npos) ? content.size() : nextNewline + 1;
            return block;
        }
        if(nextNewline == std::string_view::npos) break;  // het file ma chua co dong dong
        lines.emplace_back(line);
        currPos = nextNewline + 1;

    }
    return block; // chay ma khong thay dong "---"
}   







std::optional<std::string> getFrontmatterValue(const std::vector<std::string>& lines, std::string_view key){
    for(const std::string& line : lines){
        const auto kv = parseKeyValueLine(line);
        if(kv && kv->key == key) return std::string(kv->value);
    }
    return std::nullopt;
}




std::set<std::string> parseTagList(std::string_view rawValue){
    std::set<std::string> tags;
    std::string_view v = trimSpaces(rawValue);
    if(v.size() < 2 || v.front() != '[' || v.back() != ']') return tags;
    v = v.substr(1, v.size() - 2);

    while(true){
        const std::size_t comma = v.find(',');
        std::string_view token = unquote(trimSpaces(v.substr(0, comma)));
        const std::string tag = normalizeTag(token);     // bo '#', chu thuong, cat dau cach
        if(isValidTag(tag)) tags.insert(tag);
        if(comma == std::string_view::npos) break;
        v.remove_prefix(comma + 1);
    }
    return tags;
}



std::string setFrontmatterValue(std::string_view content,
                                std::string_view key,
                                std::string_view value){
    key = trimSpaces(key);
    if(key.empty() || key.find(':') != std::string_view::npos ||
       hasLineBreak(key) || hasLineBreak(value)){
        return std::string(content);
    }

    FrontmatterBlock block = splitFrontmatter(content);

    std::string newLine(key);
    newLine += ':';
    if(!value.empty()){
        newLine += ' ';
        newLine += value;
    }

    bool replaced = false;
    if(block.hasFrontmatter){
        for(std::string& line : block.lines){
            if(isKeyLine(line, key)){
                line = newLine;          // an toan: khong con view nao tro vao `line`
                replaced = true;
                break;
            }
        }
    }
    if(!replaced) block.lines.push_back(newLine);

    std::string result = "---\n";
    for(const std::string& line : block.lines){
        result += line;
        result += '\n';
    }
    result += "---\n";
    result += content.substr(block.bodyOffset);              // than bai: nguyen ven
    return result;
}





// Chua toi uu, can phai sua lai sau.
std::set<std::string> extractTags(std::string_view content) {
    std::set<std::string> tags;

    // Nguon 1: "tags: [...]" trong frontmatter
    const FrontmatterBlock block = splitFrontmatter(content);
    if(block.hasFrontmatter){
        if(const auto raw = getFrontmatterValue(block.lines, "tags")){
            tags = parseTagList(*raw);
        }
    }

    // Nguon 2: "#tag" trong than bai (khong quet frontmatter, khong quet ben trong [[ ]])
    const std::string_view body = content.substr(block.bodyOffset);
    std::vector<LinkSpan> links = findLinkSpans(body);

    // Cac vi tri '#' duoc hoi theo thu tu tang dan nen chi can mot chi so truot.
    // Vung cua mot link: tu "[[" den het "]]" = [innerBegin - 2, innerEnd + 2)
    std::size_t li = 0;
    auto isInLink = [&](std::size_t pos){
        while(li < links.size() && links[li].innerEnd + 2 <= pos) li++;
        return li < links.size() && pos + 2 >= links[li].innerBegin;
    };

    const std::size_t len = body.size();
    for (std::size_t i = 0; i < len; i++) {
        if (body[i] != '#') continue;
        if (i != 0 && !canPrecedeTag(body[i - 1])) continue;
        if (isInLink(i)) continue;

        // Check sau # phai la chu hoac so.
        std::size_t j = i + 1;
        if (j >= len || !std::isalnum(static_cast<unsigned char>(body[j]))) {
            continue;
        }

        std::string tag;
        while (j < len) {
            unsigned char ch = static_cast<unsigned char>(body[j]);
            if (std::isalnum(ch) || ch == '_' || ch == '-') {
                tag += static_cast<char>(std::tolower(ch));
                j++;
            } else {
                break;
            }
        }
        tags.insert(std::move(tag));
        i = j - 1;
    }
    return tags;
}
