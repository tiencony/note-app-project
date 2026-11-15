#include "test_framework.h"
#include "tests.h"
#include "Utils/Parser.h"

#include <set>

void testTags(){
    using Tags = std::set<std::string>;

    check(extractTags("hoc #cpp va #Very-important")
              == Tags{"cpp", "very-important"},
          "extractTags: lay duoc 2 tag, doi chu thuong");
    check(extractTags("# Tieu de\nnoi dung").empty(),
          "extractTags: heading '# ' khong phai tag");
    check(extractTags("abc#def").empty(),
          "extractTags: '#' dinh lien chu thi khong phai tag");
    check(extractTags("#start\nxuong dong\n#nl") == Tags{"start", "nl"},
          "extractTags: tag o dau chuoi va sau xuong dong");
    check(extractTags("#cpp #CPP #Cpp").size() == 1,
          "extractTags: trung tag chi tinh 1");
    check(extractTags("xem (#cpp) nhe") == Tags{"cpp"},
          "extractTags: tag trong ngoac don");
    check(extractTags("noi \"#cpp\" nhe") == Tags{"cpp"},
          "extractTags: tag trong ngoac kep");
    check(extractTags("http://a.com/page#section").empty(),
          "extractTags: neo URL khong phai tag");
    check(extractTags("[[Note#Heading]]").empty(),
          "extractTags: heading trong link khong phai tag");

    check(normalizeTag(" #CPP ") == "cpp",        "normalizeTag: cat dau cach");
    check(normalizeTag(normalizeTag("#A")) == normalizeTag("#A"), "normalizeTag: idempotent");
    check(normalizeTitle(normalizeTitle(" A.MD ")) == normalizeTitle(" A.MD "), "normalizeTitle: idempotent");
    check(normalizeTag("#CPP") == "cpp", "normalizeTag: bo # va doi chu thuong");
    check(normalizeTag("Cpp") == "cpp",  "normalizeTag: khong co # van dung");
}

void testLinks(){
    using Set = std::set<std::string>;

    check(extractLinks("xem [[Note A]] va [[Note B]]") == Set{"Note A", "Note B"},
          "extractLinks: lay duoc 2 link");
    check(extractLinks("[[A]] [[A]]").size() == 1,
          "extractLinks: link trung chi tinh 1");
    check(extractLinks("[[ Note ]]") == Set{"Note"},
          "extractLinks: cat khoang trang hai dau");
    check(extractLinks("[[]] [[ ]]").empty(),
          "extractLinks: ten rong bi bo qua");
    check(extractLinks("[[chua dong").empty(),
          "extractLinks: thieu ]] thi khong co link");
    check(extractLinks("[[a\nb]]").empty(),
          "extractLinks: ten co xuong dong la link hong");
    check(extractLinks("[[a [[b]]") == Set{"b"},
          "extractLinks: link hong khong nuot link dung");
    check(extractLinks("[ [x] ]").empty(),
          "extractLinks: ngoac don khong phai link");
}


void testReplaceLink(){
    check(replaceLink("xem [[A]] va [[B]]", "A", "C") == "xem [[C]] va [[B]]",
          "replaceLink: chi thay link khop, giu nguyen link khac");
    check(replaceLink("[[a]]", "A", "Z") == "[[Z]]",
          "replaceLink: khong phan biet hoa/thuong");
    check(replaceLink("[[A]] [[A]]", "A", "Z") == "[[Z]] [[Z]]",
          "replaceLink: thay moi lan xuat hien");
    check(replaceLink("[[ A ]]", "A", "Z") == "[[Z]]",
          "replaceLink: link co khoang trang hai dau van khop");
    check(replaceLink("[[B]]", "A", "Z") == "[[B]]",
          "replaceLink: khong khop thi giu nguyen");
    check(replaceLink("A [[B]] A", "A", "Z") == "A [[B]] A",
          "replaceLink: chu A ben ngoai [[ ]] khong bi dung toi");
    check(replaceLink("dong 1\n#tag [[A]]\ncuoi", "A", "Z") == "dong 1\n#tag [[Z]]\ncuoi",
          "replaceLink: giu nguyen xuong dong va tag");
    check(replaceLink("[[x [[A]]", "A", "Z") == "[[x [[Z]]",
          "replaceLink: link hong khong lam mat link dung");
    check(replaceLink("[[]]", "", "Z") == "[[]]",
          "replaceLink: ten cu rong thi khong doi gi");
}



// Ham tro giup: in gia tri thuc te khi sai
static void checkParse(const std::string& inner,
                       const std::string& target, const std::string& suffix){
    LinkParts p = parseLink(inner);
    const bool ok = (p.target == target && p.suffix == suffix);
    check(ok, "parseLink: [" + inner + "]");
    if(!ok){
        std::cout << "    thuc te : target=[" << p.target << "] suffix=[" << p.suffix << "]\n"
                  << "    mong doi: target=[" << target   << "] suffix=[" << suffix   << "]"
                  << std::endl;
    }
}

static void checkReplace(const std::string& in, const std::string& expected,
                         const std::string& name){
    const std::string got = replaceLink(in, "A", "B");
    check(got == expected, "replaceLink: " + name);
    if(got != expected){
        std::cout << "    thuc te : [" << got      << "]\n"
                  << "    mong doi: [" << expected << "]" << std::endl;
    }
}

void testParseLink(){
    checkParse("A",           "A",   "");
    checkParse(" A ",         "A",   "");
    checkParse("A|b",         "A",   "|b");
    checkParse("A#h",         "A",   "#h");
    checkParse("A#h|b",       "A",   "#h|b");       // '#' den truoc
    checkParse("A|b#c",       "A",   "|b#c");       // '|' den truoc, '#' chi la chu trong bi danh
    checkParse("A | b ",      "A",   "| b ");       // suffix giu nguyen ca dau cach
    checkParse(" A.md | x ",  "A.md","| x ");       // target chua cat duoi .md (de normalizeTitle lam)
    checkParse("A#",          "A",   "#");
    checkParse("#h",          "",    "#h");         // target rong: nhay toi muc trong chinh ghi chu
    checkParse("|b",          "",    "|b");
    checkParse("",            "",    "");
    checkParse("   ",         "",    "");
}

void testFindLinkSpans(){
    const std::string s = "xx [[A]] yy [[B|c]]";
    auto spans = findLinkSpans(s);
    check(spans.size() == 2, "findLinkSpans: 2 link");
    if(spans.size() == 2){
        check(s.substr(spans[0].innerBegin, spans[0].innerEnd - spans[0].innerBegin) == "A",
              "findLinkSpans: noi dung link thu nhat");
        check(s.substr(spans[1].innerBegin, spans[1].innerEnd - spans[1].innerBegin) == "B|c",
              "findLinkSpans: noi dung link thu hai");
    }
    check(findLinkSpans("[[a\nb]]").empty(),     "findLinkSpans: link ngat dong khong tinh");
    check(findLinkSpans("[[chua dong").empty(),  "findLinkSpans: thieu ]]");
    check(findLinkSpans("[[a [[b]]").size() == 1,"findLinkSpans: link hong khong nuot link dung");
    check(findLinkSpans("[[[A]]").size() == 1,   "findLinkSpans: ba dau [ van lay duoc link A");
    check(findLinkSpans("[[]]").size() == 1,     "findLinkSpans: span rong van duoc tra ve (extractLinks tu loc)");
}

void testLinksAlias(){
    using Set = std::set<std::string>;

    check(extractLinks("[[A|x]] [[A#h]] [[A]]") == Set{"A"},
          "extractLinks: 3 cach viet cung mot dich -> 1 phan tu");
    check(extractLinks("[[#h]] [[|x]]").empty(),
          "extractLinks: target rong bi bo qua");
    check(extractLinks("[[A#h|x]] [[B]]") == Set{"A", "B"},
          "extractLinks: link co suffix va link thuong");
    check(extractLinks("[[ A | x ]]") == Set{"A"},
          "extractLinks: cat dau cach quanh target");
    check(extractLinks("[[A.md|x]]") == Set{"A.md"},
          "extractLinks: tra ve target nguyen chinh ta (chuan hoa lam o noi khac)");
    check(extractLinks("[[C|A]]") == Set{"C"},
          "extractLinks: chu trong bi danh khong phai dich");
}

void testReplaceLinkAlias(){
    checkReplace("[[A]]",                  "[[B]]",                  "link thuong");
    checkReplace("[[A|bi danh]]",          "[[B|bi danh]]",          "giu bi danh");
    checkReplace("[[A#Muc 1]]",            "[[B#Muc 1]]",            "giu heading");
    checkReplace("[[A#Muc 1|bi danh]]",    "[[B#Muc 1|bi danh]]",    "giu heading va bi danh");
    checkReplace("[[ a.md | x ]]",         "[[B| x ]]",              "khop theo khoa, giu suffix");
    checkReplace("[[ A ]]",                "[[B]]",                  "bo dau cach trong ngoac khi khong co suffix");
    checkReplace("[[C|A]]",                "[[C|A]]",                "chu A trong bi danh khong doi");
    checkReplace("[[#A]]",                 "[[#A]]",                 "target rong khong doi");
    checkReplace("[[AB]]",                 "[[AB]]",                 "khong khop tien to");
    checkReplace("A [[A]] A",              "A [[B]] A",              "chi doi trong [[ ]]");
    checkReplace("[[A]] va [[a]] va [[A.md]]", "[[B]] va [[B]] va [[B]]", "nhieu cach viet cung dich");
    checkReplace("[[A]][[A]]",             "[[B]][[B]]",             "hai link sat nhau");
    checkReplace("[[A]]\n[[A|x]]\n",       "[[B]]\n[[B|x]]\n",       "nhieu dong");
    checkReplace("[[a\nb]]",               "[[a\nb]]",               "link hong khong doi");
    checkReplace("[[A",                    "[[A",                    "thieu ]] khong doi");
    checkReplace("",                       "",                       "chuoi rong");

    check(replaceLink("[[A]]", "", "B") == "[[A]]",    "replaceLink: ten cu rong thi khong doi");
    check(replaceLink("[[A]]", "   ", "B") == "[[A]]", "replaceLink: ten cu toan dau cach thi khong doi");

    // Doi roi, ten cu khong con, ten moi co mat
    const std::string out = replaceLink("[[A|x]] [[A#h]]", "A", "B");
    check(extractLinks(out) == std::set<std::string>{"B"},
          "replaceLink: sau khi doi chi con link toi B");
    check(replaceLink(out, "A", "B") == out,
          "replaceLink: goi lai lan hai khong doi gi them");
}

void testTagsAndLinks(){
    using Tags = std::set<std::string>;

    check(extractTags("[[Note #Muc]]").empty(),
          "extractTags: dau # co dau cach truoc, nam trong link -> khong phai tag");
    check(extractTags("[[Note#Muc]]").empty(),
          "extractTags: heading trong link khong phai tag");
    check(extractTags("#tag [[Note]] #tag2") == Tags{"tag", "tag2"},
          "extractTags: tag ngoai link van lay duoc");
    check(extractTags("[[A#h]] #real") == Tags{"real"},
          "extractTags: link co heading roi toi tag that");
    check(extractTags("[[A|#x]]").empty(),
          "extractTags: # trong bi danh khong phai tag");
}






void testSplitFrontmatter() {
    // 1. Trường hợp chuẩn (xuống dòng Unix \n)
    {
        const std::string content = "---\ncreated: 2026-11-14\ntags: [cpp, oop]\n---\nNoi dung than bai";
        FrontmatterBlock b = splitFrontmatter(content);
        check(b.hasFrontmatter, "splitFrontmatter: nhan dien khoi chuan");
        check(b.lines.size() == 2 && b.lines[0] == "created: 2026-11-14" && b.lines[1] == "tags: [cpp, oop]",
              "splitFrontmatter: tach dung cac dong ben trong");
        check(content.substr(b.bodyOffset) == "Noi dung than bai",
              "splitFrontmatter: bodyOffset tro dung dau than bai");
        // check(bodyOffset(content) == b.bodyOffset,
        //       "bodyOffset: khop voi splitFrontmatter");
    }

    // 2. Xuống dòng kiểu Windows (\r\n)
    {
        const std::string content = "---\r\ncreated: x\r\n---\r\nbody";
        FrontmatterBlock b = splitFrontmatter(content);
        check(b.hasFrontmatter, "splitFrontmatter: chiu duoc xuong dong \\r\\n");
        check(b.lines.size() == 1 && b.lines[0] == "created: x",
              "splitFrontmatter: cat bo \\r o cuoi dong");
        check(content.substr(b.bodyOffset) == "body",
              "splitFrontmatter: bodyOffset nhay qua ca \\r\\n");
    }

    // 3. Có BOM UTF-8 (\xEF\xBB\xBF) ở đầu file
    {
        const std::string content = "\xEF\xBB\xBF---\ncreated: x\n---\nbody";
        FrontmatterBlock b = splitFrontmatter(content);
        check(b.hasFrontmatter, "splitFrontmatter: nhan dien khoi khi co BOM UTF-8");
        check(b.lines.size() == 1 && b.lines[0] == "created: x",
              "splitFrontmatter: doc dung dong khi co BOM");
        check(content.substr(b.bodyOffset) == "body",
              "splitFrontmatter: bodyOffset dung khi co BOM");
    }

    // 4. Khối rỗng (--- mở và đóng ngay lập tức) và dòng đóng nằm cuối file không có \n
    {
        const std::string emptyBlock = "---\n---\nbody";
        FrontmatterBlock b1 = splitFrontmatter(emptyBlock);
        check(b1.hasFrontmatter && b1.lines.empty() && emptyBlock.substr(b1.bodyOffset) == "body",
              "splitFrontmatter: khoi frontmatter rong hop le");

        const std::string noTrailingNewline = "---\ncreated: x\n---";
        FrontmatterBlock b2 = splitFrontmatter(noTrailingNewline);
        check(b2.hasFrontmatter && b2.bodyOffset == noTrailingNewline.size(),
              "splitFrontmatter: dong dong o cuoi file khong co \\n");
    }

    // 5. Các trường hợp KHÔNG PHẢI frontmatter
    {
        const std::string notAtStart = "body\n---\ncreated: x\n---\n";
        FrontmatterBlock b1 = splitFrontmatter(notAtStart);
        check(!b1.hasFrontmatter && b1.bodyOffset == 0 && b1.lines.empty(),
              "splitFrontmatter: khoi khong nam dau file -> bo qua");

        const std::string unclosed = "---\ncreated: x\nbody";
        FrontmatterBlock b2 = splitFrontmatter(unclosed);
        check(!b2.hasFrontmatter && b2.bodyOffset == 0 && b2.lines.empty(),
              "splitFrontmatter: khong co dong dong --- -> bo qua");

        const std::string leadingSpace = " ---\ncreated: x\n---\nbody";
        FrontmatterBlock b3 = splitFrontmatter(leadingSpace);
        check(!b3.hasFrontmatter && b3.bodyOffset == 0,
              "splitFrontmatter: co dau cach truoc --- -> khong phai frontmatter");
    }

    // 6. Các chuỗi biên (rỗng, chỉ có ---, dòng bên trong chứa dấu ---)
    {
        FrontmatterBlock bEmpty = splitFrontmatter("");
        check(!bEmpty.hasFrontmatter && bEmpty.bodyOffset == 0,
              "splitFrontmatter: chuoi rong khong sap");

        FrontmatterBlock bOnlyDash = splitFrontmatter("---");
        check(!bOnlyDash.hasFrontmatter && bOnlyDash.bodyOffset == 0,
              "splitFrontmatter: chi co mot dong --- khong sap");

        const std::string dashInside = "---\ntitle: Phan 1 --- Phan 2\n---\nbody";
        FrontmatterBlock bInside = splitFrontmatter(dashInside);
        check(bInside.hasFrontmatter && bInside.lines.size() == 1 && bInside.lines[0] == "title: Phan 1 --- Phan 2",
              "splitFrontmatter: dau --- nam giua dong khong bi hieu nham la dong dong");
    }
}