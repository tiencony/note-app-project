#include "Utils/StringUtils.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>

std::string_view trimSpaces(std::string_view s) {
    const std::size_t begin = s.find_first_not_of(" \t");
    if (begin == std::string_view::npos) return {};

    const std::size_t end = s.find_last_not_of(" \t");
    return s.substr(begin, end - begin + 1);
}

bool equalsIgnoreCase(std::string_view str1, std::string_view str2) {
    if (str1.size() != str2.size()) return false;

    return std::equal(
        str1.begin(), str1.end(),
        str2.begin(), str2.end(),
        [](unsigned char ch1, unsigned char ch2) {
            return std::tolower(ch1) == std::tolower(ch2);
        }
    );
}

std::size_t findIgnoreCase(std::string_view text, std::string_view keyword) {
    auto it = std::search(
        text.begin(), text.end(),
        keyword.begin(), keyword.end(),
        [](unsigned char a, unsigned char b) {
            return std::tolower(a) == std::tolower(b);
        }
    );

    if (it == text.end()) return std::string_view::npos;
    return static_cast<std::size_t>(it - text.begin());
}

bool containsIgnoreCase(std::string_view text, std::string_view keyword) {
    return findIgnoreCase(text, keyword) != std::string_view::npos;
}

std::string makeSnippet(std::string_view text, std::size_t pos, std::size_t keywordLen) {
    const std::size_t margin = 20;
    const std::size_t start = (pos > margin) ? pos - margin : 0;
    const std::size_t end = std::min(text.size(), pos + keywordLen + margin);

    std::string snippet(text.substr(start, end - start));
    std::replace(snippet.begin(), snippet.end(), '\n', ' ');

    const std::size_t keywordPosInSnippet = (pos > margin) ? margin : pos;

    snippet.insert(keywordPosInSnippet + keywordLen, "\"");
    snippet.insert(keywordPosInSnippet, "\"");

    if (start > 0) snippet = "..." + snippet;
    if (end < text.size()) snippet += "...";

    return snippet;
}
