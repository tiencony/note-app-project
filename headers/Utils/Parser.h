
#ifndef PARSER_H
#define PARSER_H

#include <set>
#include <string>
#include <vector>
#include <string_view>
#include <optional>


// Khoa cua tag: cat dau cach -> bo '#' dau -> chu thuong
std::string normalizeTag(std::string_view tag);
// Khoa cua tieu de / link: cat dau cach -> chu thuong -> bo duoi ".md"
std::string normalizeTitle(std::string_view s);


struct LinkSpan{
    std::size_t innerBegin;  // vi tri ngay sau "[["
    std::size_t innerEnd;    // vi tri cua "]]"
};


struct LinkParts{
    std::string_view target;  // da cat dau cach
    std::string_view suffix;  // tu ky tu '|' hoac '#' day tien tro di, giu nguyen van
};

LinkParts parseLink(std::string_view inner);

std::set<std::string> extractTags(std::string_view content);
std::vector<LinkSpan> findLinkSpans(std::string_view content);
std::set<std::string> extractLinks(std::string_view content);

std::string replaceLink(
    std::string_view content,
    std::string_view oldTitle,
    std::string_view newTitle
);







// DIEU KIEN: content da duoc chuan hoa luc doc file (readFileToString): khong co BOM,
// xuong dong chi la "\n". Parser khong tu xu ly BOM hay '\r'.
struct FrontmatterBlock{
    bool hasFrontmatter = false;
    std::size_t bodyOffset = 0;      // vi tri bat dau body (khong co khoi: = 0)
    std::vector<std::string> lines;  // cac dong ben trong khoi, da cat '\r'
};

FrontmatterBlock splitFrontmatter(std::string_view content);

// nullopt = khong co khoa; "" (co gia tri) = khoa ton tai nhung de trong
std::optional<std::string> getFrontmatterValue(const std::vector<std::string>& lines,
                                               std::string_view key);

// "[a, B, #c]" -> {a, b, c}. Chi nhan dang co ngoac vuong.
std::set<std::string> parseTagList(std::string_view rawValue);

// Dat gia tri cho mot khoa, giu nguyen cac dong khac. Dau vao khong hop le
// (khoa rong, khoa chua ':', khoa/gia tri chua xuong dong) -> tra nguyen content.
std::string setFrontmatterValue(
    std::string_view content,
    std::string_view targetKey,
    std::string_view newValue
);


// Hop cua tag trong "tags: [...]" (frontmatter) va tag "#..." trong body
std::set<std::string> extractTags(std::string_view content);

#endif
