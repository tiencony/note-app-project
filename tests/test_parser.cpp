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