#include "test_framework.h"
#include "tests.h"
#include "NoteManager/NoteManager.h"
#include "Utils/Utils.h"
#include <filesystem>
#include <string>
#include <system_error>
#include <vector>
#include <fstream>

namespace fs = std::filesystem;

// Xoa moi file zz_test* trong vault
void cleanupTestFiles(){
    std::error_code ec;
    std::vector<fs::path> toDelete;
    for(const auto& e : fs::directory_iterator("vault/notes", ec)){
        if(normalizeTitle(e.path().filename().string()).rfind("zz_test", 0) == 0){
            toDelete.push_back(e.path());
        }
    }
    for(const auto& p : toDelete) fs::remove(p, ec);
}

void testCrud(){
    cleanupTestFiles();
    NoteManager m;

    // Tao
    check(m.createNote("zz_test_A", "hoc con tro C++"), "Tao note A");
    check(m.createNote("zz_test_B", "ke hoach tuan"),   "Tao note B");
    check(m.createNote("zz_test_C", "ghi chu khac"),    "Tao note C");

    // Kiem tra tieu de
    check(!m.createNote("ZZ_TEST_a", "x"), "Tu choi trung ten khac hoa/thuong");
    check(!m.createNote("a/b", "x"),       "Tu choi ten chua '/'");
    check(!m.createNote("", "x"),          "Tu choi ten rong");
    check(!m.createNote("abc.", "x"),      "Tu choi ten ket thuc bang dau cham");

    // Tim kiem
    check(m.searchTitle("TEST_A").size() == 1,        "searchTitle khong phan biet hoa/thuong");
    check(m.searchTitle("zz_test_").size() == 3,      "searchTitle ra du 3 note");
    check(m.searchTitle("khong_co").empty(),          "searchTitle khong tim thay");
    check(m.searchContent("CON TRO").size() == 1,     "searchContent tim thay");
    check(m.searchContent("khong_co_tu_nay").empty(), "searchContent khong tim thay");

    // Mo / sua
    check(m.openNote("zz_test_a"),      "Mo note (go khac hoa/thuong)");
    check(!m.openNote("khong_ton_tai"), "Mo note khong ton tai tra ve false");
    check(m.editNote("zz_test_A", "noi dung moi hoan toan"), "Sua note A");
    check(m.searchContent("hoan toan").size() == 1, "Noi dung moi tim duoc");
    check(m.searchContent("con tro").empty(),       "Noi dung cu khong con");


    // Doi ten

    check(m.renameNote("zz_test_B", "zz_test_B2"),  "Doi ten B -> B2");
    check(!m.openNote("zz_test_B"),                 "Ten cu khong con");
    check(m.openNote("zz_test_B2"),                 "Ten moi mo duoc");
    check(!m.renameNote("zz_test_B2", "zz_test_C"), "Tu choi doi ten trung note khac");
    check(m.renameNote("zz_test_C", "ZZ_TEST_C"),   "Cho phep doi hoa/thuong chinh no");

    // Doc lai tu dia bang doi tuong moi
    NoteManager m2;
    m2.loadNotesFromDisk();
    check(m2.searchTitle("zz_test_").size() == 3,    "Load lai tu dia thay du 3 note");
    check(m2.searchContent("hoan toan").size() == 1, "Noi dung sua da duoc luu xuong dia");

    // Xoa
    check(m.deleteNote("zz_test_A"),  "Xoa A (phan tu dau)");
    check(m.deleteNote("zz_test_C"),  "Xoa C");
    check(m.deleteNote("zz_test_B2"), "Xoa B2 (phan tu cuoi)");
    check(!m.deleteNote("zz_test_A"), "Xoa lai note da xoa tra ve false");
    check(m.searchTitle("zz_test_").empty(), "Khong con note nao");

    // Goi load hai lan khong bi nhan doi
    NoteManager r;
    r.createNote("zz_test_R", "x");
    r.loadNotesFromDisk();
    check(r.searchTitle("zz_test_R").size() == 1,
          "loadNotesFromDisk: tai lai khong nhan doi note");

    // cleanupTestFiles();
}

void testBacklinks(){
    cleanupTestFiles();
    NoteManager b;
    b.createNote("zz_test_L1", "xem [[zz_test_L2]]");
    b.createNote("zz_test_L2", "khong co link nao");
    b.createNote("zz_test_L3", "[[ZZ_TEST_L2]] va [[zz_test_L2]]");
    b.createNote("zz_test_L4", "[[zz_test_L4]] tu tro vao minh");

    check(b.backlinks("zz_test_L2").size() == 2,
          "backlinks: L1 va L3 tro toi L2 (L3 chi tinh 1 lan)");
    check(b.backlinks("zz_test_l2").size() == 2,
          "backlinks: khong phan biet hoa/thuong");
    check(b.backlinks("zz_test_L1").empty(),
          "backlinks: khong ai tro toi L1");
    check(b.backlinks("zz_test_L4").empty(),
          "backlinks: link tro vao chinh no khong tinh");
    check(b.backlinks("khong_ton_tai").empty(),
          "backlinks: note khong ton tai thi rong");

    cleanupTestFiles();
}

void testBrokenLinks(){
    cleanupTestFiles();
    NoteManager b;
    check(b.createNote("zz_test_L1", "[[zz_test_L2]] va [[zz_test_khong_co]]"),
          "brokenLinks: tao L1");
    check(b.createNote("zz_test_L2", "noi dung"),
          "brokenLinks: tao L2");

    auto broken = b.brokenLinks();
    check(broken.size() == 1, "brokenLinks: chi 1 link hong");
    check(!broken.empty() && broken[0].from == "zz_test_L1"
                          && broken[0].target == "zz_test_khong_co",
          "brokenLinks: dung note chua link va dung ten hong");

    b.deleteNote("zz_test_L2");
    check(b.brokenLinks().size() == 2,
          "brokenLinks: xoa L2 thi link toi L2 thanh hong");

    b.deleteNote("zz_test_L1");
    check(b.brokenLinks().empty(), "brokenLinks: het note thi het link hong");

    cleanupTestFiles();
}

void testLinkIndexFresh(){
    cleanupTestFiles();
    NoteManager b;
    b.createNote("zz_test_X", "[[zz_test_Y]]");
    check(b.brokenLinks().size() == 1, "index: X tro toi Y chua co -> 1 link hong");

    b.createNote("zz_test_Y", "y");
    check(b.brokenLinks().empty(),              "index: tao Y -> het hong (tu lam moi)");
    check(b.backlinks("zz_test_Y").size() == 1, "index: Y co 1 backlink tu X");

    b.editNote("zz_test_X", "khong con link");
    check(b.backlinks("zz_test_Y").empty(),     "index: sua X bo link -> Y het backlink");

    b.deleteNote("zz_test_Y");
    b.deleteNote("zz_test_X");
    check(b.brokenLinks().empty(), "index: xoa het -> rong");

    cleanupTestFiles();
}


void testBrokenLinksDedup(){
    cleanupTestFiles();
    NoteManager b;
    b.createNote("zz_test_D1", "[[zz_test_ghost]] va [[ZZ_TEST_GHOST]] va [[zz_test_ghost.md]]");

    check(b.brokenLinks().size() == 1,
          "brokenLinks: cung mot dich viet khac hoa/thuong chi bao 1 lan");

    cleanupTestFiles();
}


void testRenameUpdatesLinks(){
    cleanupTestFiles();
    NoteManager m;
    m.createNote("zz_test_Old", "tu tro [[zz_test_Old]]");
    m.createNote("zz_test_A", "xem [[zz_test_Old]]");
    m.createNote("zz_test_B", "xem [[ ZZ_TEST_OLD.md ]]");
    m.createNote("zz_test_C", "khong lien quan");

    check(m.renameNote("zz_test_Old", "zz_test_New"), "rename: doi ten");
    check(m.brokenLinks().empty(),                    "rename: khong con link hong");
    check(m.backlinks("zz_test_New").size() == 2,     "rename: A va B tro toi ten moi");
    check(m.searchContent("[[zz_test_New]]").size() == 3,
          "rename: A, B va chinh no deu duoc viet lai");

    // Sua file ngoai chuong trinh, roi doi ten: khong duoc mat noi dung
    {
        std::ofstream out("vault/notes/zz_test_C.md");
        out << "them ngoai chuong trinh [[zz_test_New]]";
    }
    check(m.renameNote("zz_test_New", "zz_test_Newer"), "rename: lan 2");
    check(m.searchContent("them ngoai chuong trinh").size() == 1,
          "rename: noi dung sua ngoai chuong trinh khong bi ghi de");
    check(m.brokenLinks().empty(), "rename: C (sua ngoai) cung duoc cap nhat link");

    cleanupTestFiles();
}
