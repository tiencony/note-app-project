#include "Utils/StringUtils.h"

#include <algorithm>
#include <cctype>


std::string trimSpaces(const std::string& s){
    std::size_t begin = s.find_first_not_of(' ');
    if(begin == std::string::npos) return "";
    std::size_t end = s.find_last_not_of(' ');
    return s.substr(begin, end - begin + 1);
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

