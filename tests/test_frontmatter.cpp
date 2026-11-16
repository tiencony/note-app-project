#include "test_framework.h"
#include "tests.h"
#include "Utils/Parser.h"
#include "Utils/StringUtils.h"

#include "Utils/FileUtils.h"

#include <filesystem>
#include <fstream>
#include <set>
#include <string>

namespace {
using Tags = std::set<std::string>;

std::string bodyOf(const std::string& content){
    return content.substr(splitFrontmatter(content).bodyOffset);
}
}

void testSplitFrontmatter(){
    FrontmatterBlock b = splitFrontmatter("---\ncreated: x\n---\nbody");
    check(b.hasFrontmatter, "split: co khoi");
    check(b.lines.size() == 1 && b.lines[0] == "created: x", "split: noi dung khoi");
    check(bodyOf("---\ncreated: x\n---\nbody") == "body", "split: bodyOffset dung");

    check(!splitFrontmatter("body\n---\nk: x\n---\n").hasFrontmatter, "split: khoi khong o dau file thi khong phai");
    b = splitFrontmatter("---\nk: x\nbody");
    check(!b.hasFrontmatter && b.bodyOffset == 0, "split: khong co dong dong -> khong phai frontmatter");
    check(!splitFrontmatter(" ---\nk: x\n---\n").hasFrontmatter, "split: dau cach truoc --- thi khong phai");
    check(!splitFrontmatter("").hasFrontmatter,     "split: chuoi rong");
    check(!splitFrontmatter("---").hasFrontmatter,  "split: chi co ---");
    check(!splitFrontmatter("---\n").hasFrontmatter,"split: --- roi het");

    b = splitFrontmatter("---\n---\nbody");
    check(b.hasFrontmatter && b.lines.empty() && bodyOf("---\n---\nbody") == "body", "split: khoi rong");

    b = splitFrontmatter("---\nk: v\n---");
    check(b.hasFrontmatter && bodyOf("---\nk: v\n---").empty(), "split: dong dong o cuoi file khong co xuong dong");
}

void testFrontmatterValues(){
    auto lines = splitFrontmatter("---\ncreated: 2026-11-14 10:30:00\nempty:\nquoted: \"hi\"\n"
                                  "created_by: me\nmeta:\n  nested: 1\n- item\n# comment: x\n---\n").lines;

    auto c = getFrontmatterValue(lines, "created");
    check(c && *c == "2026-11-14 10:30:00", "value: giu nguyen dau ':' trong gia tri (chi cat o ':' dau tien)");
    auto e = getFrontmatterValue(lines, "empty");
    check(e.has_value() && e->empty(), "value: khoa rong khac khong co khoa");
    check(!getFrontmatterValue(lines, "missing").has_value(), "value: khong co khoa");
    auto q = getFrontmatterValue(lines, "quoted");
    check(q && *q == "hi", "value: bo ngoac kep");
    auto cb = getFrontmatterValue(lines, "created_by");
    check(cb && *cb == "me", "value: created_by la khoa rieng, khong lan voi created");
    check(!getFrontmatterValue(lines, "nested").has_value(), "value: khoa long nhau khong phai khoa cap ngoai");
    check(!getFrontmatterValue(lines, "comment").has_value(), "value: dong comment khong phai khoa");
    check(!getFrontmatterValue(lines, "item").has_value(),    "value: muc danh sach khong phai khoa");
}

void testParseTagList(){
    check(parseTagList("[a, B , c]") == Tags{"a", "b", "c"}, "parseTagList: co ban, chuan hoa chu thuong");
    check(parseTagList("[a, a, A]").size() == 1, "parseTagList: trung sau khi chuan hoa");
    check(parseTagList("[]").empty(), "parseTagList: rong");
    check(parseTagList("[ , ]").empty(), "parseTagList: chi co dau phay");
    check(parseTagList("").empty(), "parseTagList: chuoi rong");
    check(parseTagList("[CPP, a b, #x, \"y\"]") == Tags{"cpp", "x", "y"},
          "parseTagList: bo tag co dau cach, nhan # va ngoac kep");
    check(parseTagList("a, b").empty(), "parseTagList: thieu ngoac vuong thi khong hieu (theo quy uoc)");
    check(parseTagList("[hoc-tap, x_y]") == Tags{"hoc-tap", "x_y"}, "parseTagList: cho phep - va _");
    check(parseTagList("[-a, _b]").empty(), "parseTagList: ky tu dau phai la chu/so (cung luat voi than bai)");
    check(parseTagList("[a/b]").empty(), "parseTagList: ky tu la thi bo tag");
}

void testSetFrontmatter(){
    check(setFrontmatterValue("---\nk: v\n---\nbody", "k", "w") == "---\nk: w\n---\nbody",
          "set: thay dung dong");
    check(setFrontmatterValue("---\n# c\naliases: [x]\n---\nb", "k", "1")
              == "---\n# c\naliases: [x]\nk: 1\n---\nb",
          "set: giu dong la, them khoa o cuoi khoi");
    check(setFrontmatterValue("body", "k", "1") == "---\nk: 1\n---\nbody", "set: chua co khoi thi tao moi");
    check(setFrontmatterValue("", "k", "1") == "---\nk: 1\n---\n", "set: noi dung rong");

    const std::string once = setFrontmatterValue("body", "k", "1");
    check(setFrontmatterValue(once, "k", "1") == once, "set: idempotent");


    check(setFrontmatterValue("---\ncreated_by: x\n---\n", "created", "2")
              == "---\ncreated_by: x\ncreated: 2\n---\n",
          "set: chi khop ca ten khoa, khong khop tien to");
    check(setFrontmatterValue("---\nmeta:\n  k: old\n---\n", "k", "new")
              == "---\nmeta:\n  k: old\nk: new\n---\n",
          "set: khong dung toi khoa long nhau");
    check(setFrontmatterValue("---\nk: v\n---\nb", "k", "") == "---\nk:\n---\nb", "set: gia tri rong");

    check(setFrontmatterValue("body", "k", "a\nb") == "body", "set: gia tri co xuong dong bi tu choi");
    check(setFrontmatterValue("body", "k:x", "1") == "body", "set: khoa chua ':' bi tu choi");
    check(setFrontmatterValue("body", "  ", "1") == "body",  "set: khoa rong bi tu choi");

    const std::string result = setFrontmatterValue("---\ntags: [a]\n---\n# Tieu de\n---\nnoi dung\n", "created", "2026-11-14");
    check(result == "---\ntags: [a]\ncreated: 2026-11-14\n---\n# Tieu de\n---\nnoi dung\n",
          "set: than bai (ke ca dong --- trong than bai) nguyen ven");
}

void testTagsFromFrontmatter(){
    check(extractTags("---\ntags: [CPP]\n---\n#cpp") == Tags{"cpp"}, "tags: frontmatter va than bai gop lai thanh 1");
    check(extractTags("---\ntags: [cpp, a b]\n---\n#oop") == Tags{"cpp", "oop"}, "tags: hai nguon, bo tag sai");
    check(extractTags("---\n# ghi chu #khong_phai_tag\n---\n").empty(), "tags: comment YAML khong sinh tag");
    check(extractTags("---\nnote: #abc\n---\n").empty(), "tags: '#abc' trong gia tri khoa khac khong phai tag");
    check(extractTags("---\ntags: [x]\n---\n[[A#h]] #real") == Tags{"x", "real"}, "tags: ket hop voi link trong than bai");
    check(extractTags("---\nk: v\nbody #t") == Tags{"t"}, "tags: khong co dong dong -> khong phai frontmatter, ca file la than bai");
}

void testLinksInFrontmatter(){
    check(extractLinks("---\nrelated: [[Y]]\n---\nbody") == std::set<std::string>{"Y"},
          "link: link trong frontmatter van tinh");
    check(replaceLink("---\nrelated: [[Y|x]]\n---\nxem [[Y]]", "Y", "Z")
              == "---\nrelated: [[Z|x]]\n---\nxem [[Z]]",
          "link: doi ten cap nhat ca link trong frontmatter");
}

void testSnippetCR(){
    const std::string s = makeSnippet("dong mot\r\ndong hai", 10, 4);
    check(s.find('\r') == std::string::npos && s.find('\n') == std::string::npos,
          "snippet: khong con \\r hay \\n");
}

// Chuan hoa BOM va CRLF xay ra DUNG MOT CHO: luc doc file
void testReadFileNormalizes(){
    namespace fs = std::filesystem;
    const fs::path p = fs::temp_directory_path() / "notes_norm_test.md";
    {
        std::ofstream out(p, std::ios::binary);                  // binary de ghi dung tung byte
        out << "\xEF\xBB\xBF" "---\r\ntags: [a]\r\n---\r\n#b\r\nhet";
    }
    std::string content;
    check(readFileToString(p, content), "readFile: doc duoc file");
    check(content == "---\ntags: [a]\n---\n#b\nhet", "readFile: bo BOM va CRLF -> LF");
    check(extractTags(content) == Tags{"a", "b"}, "readFile: sau chuan hoa, frontmatter va tag nhan ra dung");
    fs::remove(p);
}