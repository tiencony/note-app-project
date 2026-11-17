// ============================================================================
// Parser.h - Phan tich van ban Markdown cua ghi chu (thuan ham, khong I/O)
//
// VAI TRO   : Tach [[link]], #tag, frontmatter "---" va chuan hoa key
//             (normalizeTitle / normalizeTag) tu mot chuoi noi dung.
// KHONG LAM : Khong doc/ghi file, khong biet NoteManager hay duong dan.
// DUOC DUNG : NoteManager (index link, tag, rename).
//
// GIA DINH  : content da qua readFileToString: khong BOM, xuong dong la "\n".
//             Parser KHONG tu xu ly BOM/'\r'.
// SO HUU    : LinkSpan chi la OFFSET (khong phai string_view) de khong bi treo
//             khi chuoi goc bi sua. LinkParts so huu std::string.
//             Moi ham nhan string_view chi dung trong thoi gian goi ham;
//             KHONG luu lai.
// LUU Y     : Hien chi dung tot cho ASCII (so sanh hoa/thuong theo byte).
//             Chua bo qua vung code block / inline code. (xem TODO)
// ============================================================================



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


// So huu chuoi (std::string), KHONG phai view: co the tra ve / luu lai thoai mai,
// khong phu thuoc vao chuoi `inner` da dung de tach.
struct LinkParts {
    std::string target;   // da cat dau cach
    std::string suffix;   // tu ky tu '|' hoac '#' dau tien tro di, giu nguyen van
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
