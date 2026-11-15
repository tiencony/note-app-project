#include <string>
#include <vector>



struct FrontmatterBlock{
    bool hasFrontmatter = false;               // Kiem tra su ton tai cua khoi frontmatter
    std::size_t bodyOffset = 0;                // Index bat dau cua than bai ngay sau --- va ky tu xuong dong
    std::vector<std::string> lines;            // Danh sach tung dong ben trong cap dau ---
};

// Cat \r o cuoi dong
static std::string& stripCR(std::string& s){
    if(!s.empty() && s.back() == '\r'){
        s.pop_back();
    }
    return s;
}

FrontmatterBlock splitFrontmatter(const std::string& content){
    FrontmatterBlock block;
    std::size_t currPos = 0;

    // Bo qua BOM UTF-8 neu co o dau file
    if (content.size() >= 3 && 
            static_cast<unsigned char>(content[0]) == 0xEF &&
            static_cast<unsigned char>(content[1]) == 0xBB &&
            static_cast<unsigned char>(content[2]) == 0xBF) {
            currPos = 3;
        }
    
    std::size_t firstNewLine = content.find('\n', currPos);
    if(firstNewLine == std::string::npos) return block;

    std::string firstLine = stripCR(content.substr(currPos, firstNewLine - currPos));
    if(firstLine != "---") return block; // dong dau khong phai ---  -> khong co frontmatter

    currPos = firstNewLine + 1;
    std::vector<std::string> tempLines;

    while(currPos <= content.size()){
        std::size_t nextNewline = content.find('\n', currPos);
        std::string rawLine;
        std::size_t nextPos;
        
        if(nextNewline == std::string::npos){
            // toi dong cuoi cung cua file(ko co \n o cuoi)
            rawLine = content.substr(currPos);
            nextPos = content.size() + 1; // danh dau ket thuc vong lap
        }else{
            rawLine = content.substr(currPos, nextNewline - currPos);
            nextPos = nextNewline + 1;
        }

        std::string cleanLine = stripCR(rawLine);
        if(cleanLine == "---"){ // neu gap dong chi co "---"
            block.hasFrontmatter = true;
            block.lines = std::move(tempLines);
            // bodyOffset tro ve sau \n hoac cuoi file neu khong co \n
            block.bodyOffset = (nextNewline == std::string::npos) ? content.size() : (nextNewline + 1);
            return block;
        }
        tempLines.push_back(cleanLine);
        if(nextNewline == std::string::npos) break;
        currPos = nextPos;

    }
    return block; // chay ma khong thay dong "---"
}   



