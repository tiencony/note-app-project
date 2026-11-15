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



// Cat \r o cuoi dong.
std::string_view stripCR(std::string_view s) {
    if (!s.empty() && s.back() == '\r') {
        s.remove_suffix(1);
    }
    return s;
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
}


// Chua toi uu, can phai sua lai sau.
std::set<std::string> extractTags(std::string_view content) {
    std::set<std::string> tags;
    FrontmatterBlock block = splitFrontmatter(content);
    std::string_view body = content.substr(block.bodyOffset);

    if (block.hasFrontmatter) {
        std::string_view rawTags = getFrontmatterValue(block.lines, "tags");
        tags = parseTagList(rawTags);
    }

    std::vector<LinkSpan> links = findLinkSpans(body);

    auto isInLink = [&](std::size_t pos) {
        for (const auto& link : links) {
            // Vung link tinh tu dau [ dau tien den dau ] cuoi cung.
            if (pos + 2 >= link.innerBegin && pos <= link.innerEnd + 2) {
                return true;
            }
        }
        return false;
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
        result += parts.suffix;


        lastCopiedPos = s.innerEnd + 2; // Cap nhat khi da quet xong sau ]]
    }
    // Noi phan duoi con lai
    result += content.substr(lastCopiedPos);
    return result;
}









// Chua sua xong
FrontmatterBlock splitFrontmatter(std::string_view content){
    FrontmatterBlock block;
    std::size_t currPos = 0;

    // Bo qua BOM UTF-8 neu co o dau file
    if (content.size() >= 3 && 
        static_cast<unsigned char>(content[0]) == 0xEF &&
        static_cast<unsigned char>(content[1]) == 0xBB &&
        static_cast<unsigned char>(content[2]) == 0xBF){
        block.bomLen = 3;
        currPos = 3;
    }

    block.bodyOffset = block.bomLen;
    
    // Dong dau phai la "---"
    std::size_t firstNewLine = content.find('\n', currPos);
    if(firstNewLine == std::string_view::npos) return block;

    block.eol = (firstNewLine > currPos && content[firstNewLine - 1] == '\r') ? "\r\n" : "\n"; 
    std::string_view firstLine = stripCR(content.substr(currPos, firstNewLine - currPos));
    if(firstLine != "---") return block; // dong dau khong phai ---  -> khong co frontmatter


    currPos = firstNewLine + 1;
    std::vector<std::string> tempLines;

    while(currPos <= content.size()){
        std::size_t nextNewline = content.find('\n', currPos);
        std::string_view rawLine;
        std::size_t nextPos;
        
        if(nextNewline == std::string::npos){
            // toi dong cuoi cung cua file(ko co \n o cuoi)
            rawLine = content.substr(currPos);
            nextPos = content.size() + 1; // danh dau ket thuc vong lap
        }else{
            rawLine = content.substr(currPos, nextNewline - currPos);
            nextPos = nextNewline + 1;
        }

        std::string_view cleanLine = stripCR(rawLine);
        if(cleanLine == "---"){ // neu gap dong chi co "---"
            block.hasFrontmatter = true;
            block.lines = std::move(tempLines);
            // bodyOffset tro ve sau \n hoac cuoi file neu khong co \n
            block.bodyOffset = (nextNewline == std::string::npos) ? content.size() : (nextNewline + 1);
            return block;
        }
        tempLines.emplace_back(cleanLine);
        if(nextNewline == std::string::npos) break;
        currPos = nextPos;

    }
    return block; // chay ma khong thay dong "---"
}   







// chua sua xong
std::string_view getFrontmatterValue(const std::vector<std::string>& lines, std::string_view targetKey){
    for(const std::string& line : lines){
        auto [key, value] = parseKeyValueLine(line);
        if(!key.empty() && key == targetKey){
            return value;
        }
    }
    return "";
}




// chua sua xong
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


std::string setFrontmatterValue(std::string_view content,  std::string_view targetKey,  std::string_view newValue){
    FrontmatterBlock block = splitFrontmatter(content);
    std::string newLine;
    newLine.reserve(targetKey.size() + 2 + newValue.size());
    appendView(newLine, targetKey);
    newLine += ": ";
    appendView(newLine, newValue);

    std::string body(content.substr(block.bodyOffset));

    
    // chua co frontmatter -> tao moi voi 1 dong
    if(!block.hasFrontmatter){
        block.lines.push_back(newLine);
    }else{
        // da co khoi frontmatter -> tim xem targetKey da co chx
        bool updated = false;
        for(std::string& line : block.lines){
            auto [key, val] = parseKeyValueLine(line);
            if(!key.empty() && key == targetKey){
                line = newLine;
                updated = true;
                break;
            }
        }
        if(!updated){
            block.lines.push_back(newLine);
        }
    }
    // lap rap thanh chuoi hoan chinh
    std::string result = "---\n";
    for(const std::string& line : block.lines){
        result += line + "\n";
    }
    result += "---\n";
    result += body;

    return result;
}