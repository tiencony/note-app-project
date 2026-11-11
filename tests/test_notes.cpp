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
static const std::filesystem::path TEST_DIR = "vault_test";

static void printLoadReport(const LoadReport& r){
    if(!r.ok){
        std::cout << "Khong doc duoc thu muc ghi chu. Giu nguyen du lieu hien co.\n";
        return;
    }
    std::cout << "Da tai " << r.loaded << " ghi chu.\n";
    for(const auto& f : r.skipped)
        std::cout << "  Bo qua (khong doc duoc): " << f << "\n";
}






// Xoa moi file zz_test* 
void cleanupTestFiles(){
    std::error_code ec;
    std::vector<fs::path> toDelete;

    for (const auto& e : fs::recursive_directory_iterator(TEST_DIR, ec)) {
        if (ec) break; 

        std::string filename = e.path().filename().string();

        if (normalizeTitle(filename).rfind("zz_test", 0) == 0) {
            toDelete.push_back(e.path());
        }
    }

    for (const auto& p : toDelete) {
        fs::remove_all(p, ec);
    }
}

void testCrud(){
    cleanupTestFiles();
    NoteManager m(TEST_DIR);

    // Tao
    check(m.createNote("zz_test_A", "hoc con tro C++") == NoteError::Ok, "Tao note A");
    check(m.createNote("zz_test_B", "ke hoach tuan") == NoteError::Ok,   "Tao note B");
    check(m.createNote("zz_test_C", "ghi chu khac") == NoteError::Ok,    "Tao note C");

    // Kiem tra tieu de
    check(m.createNote("ZZ_TEST_a", "x") == NoteError::AlreadyExists, "Tu choi trung ten khac hoa/thuong");
    check(m.createNote("a/b", "x")       == NoteError::InvalidTitle,  "Tu choi ten chua '/'");
    check(m.createNote("", "x")          == NoteError::InvalidTitle,  "Tu choi ten rong");
    check(m.createNote("abc.", "x")      == NoteError::InvalidTitle,  "Tu choi ten ket thuc bang dau cham");
    // Tim kiem
    check(m.searchTitle("TEST_A").size() == 1,        "searchTitle khong phan biet hoa/thuong");
    check(m.searchTitle("zz_test_").size() == 3,      "searchTitle ra du 3 note");
    check(m.searchTitle("khong_co").empty(),          "searchTitle khong tim thay");
    check(m.searchContent("CON TRO").size() == 1,     "searchContent tim thay");
    check(m.searchContent("khong_co_tu_nay").empty(), "searchContent khong tim thay");

    // Mo / sua
    check(m.openNote("zz_test_a").error == NoteError::Ok,        "Mo note (go khac hoa/thuong)");   
    check(m.openNote("khong_ton_tai").error == NoteError::NotFound, "Mo note khong ton tai");
    check(m.editNote("zz_test_A", "noi dung moi hoan toan") == NoteError::Ok, "Sua note A");
    check(m.searchContent("hoan toan").size() == 1, "Noi dung moi tim duoc");
    check(m.searchContent("con tro").empty(),       "Noi dung cu khong con");


    // Doi ten

    check(m.renameNote("zz_test_B", "zz_test_B2") == NoteError::Ok,  "Doi ten B -> B2");
    check(m.openNote("zz_test_B").error == NoteError::NotFound,                 "Ten cu khong con");
    check(m.openNote("zz_test_B2").error == NoteError::Ok,                 "Ten moi mo duoc");
    check(m.renameNote("zz_test_B2", "zz_test_C") != NoteError::Ok, "Tu choi doi ten trung note khac");
    check(m.renameNote("zz_test_C", "ZZ_TEST_C") == NoteError::Ok,   "Cho phep doi hoa/thuong chinh no");

    // Doc lai tu dia bang doi tuong moi
    NoteManager m2(TEST_DIR);
    printLoadReport(m2.loadNotesFromDisk());
    check(m2.searchTitle("zz_test_").size() == 3,    "Load lai tu dia thay du 3 note");
    check(m2.searchContent("hoan toan").size() == 1, "Noi dung sua da duoc luu xuong dia");

    // Xoa
    check(m.deleteNote("zz_test_A") == NoteError::Ok,  "Xoa A (phan tu dau)");
    check(m.deleteNote("zz_test_C") == NoteError::Ok,  "Xoa C");
    check(m.deleteNote("zz_test_B2") == NoteError::Ok, "Xoa B2 (phan tu cuoi)");
    check(m.deleteNote("zz_test_A") != NoteError::Ok, "Xoa lai note da xoa tra ve false");
    check(m.searchTitle("zz_test_").empty(), "Khong con note nao");

    // Goi load hai lan khong bi nhan doi
    NoteManager r(TEST_DIR);
    r.createNote("zz_test_R", "x");
    printLoadReport(r.loadNotesFromDisk());
    check(r.searchTitle("zz_test_R").size() == 1,
          "loadNotesFromDisk: tai lai khong nhan doi note");

    // cleanupTestFiles();
}

void testBacklinks(){
    cleanupTestFiles();
    NoteManager b(TEST_DIR);
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
    NoteManager b(TEST_DIR);
    check(b.createNote("zz_test_L1", "[[zz_test_L2]] va [[zz_test_khong_co]]") == NoteError::Ok,
          "brokenLinks: tao L1");
    check(b.createNote("zz_test_L2", "noi dung") == NoteError::Ok,
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
    NoteManager b(TEST_DIR);
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
    NoteManager b(TEST_DIR);
    b.createNote("zz_test_D1", "[[zz_test_ghost]] va [[ZZ_TEST_GHOST]] va [[zz_test_ghost.md]]");

    check(b.brokenLinks().size() == 1,
          "brokenLinks: cung mot dich viet khac hoa/thuong chi bao 1 lan");

    cleanupTestFiles();
}


void testRenameUpdatesLinks(){
    cleanupTestFiles();
    NoteManager m(TEST_DIR);
    m.createNote("zz_test_Old", "tu tro [[zz_test_Old]]");
    m.createNote("zz_test_A", "xem [[zz_test_Old]]");
    m.createNote("zz_test_B", "xem [[ ZZ_TEST_OLD.md ]]");
    m.createNote("zz_test_C", "khong lien quan");

    check(m.renameNote("zz_test_Old", "zz_test_New") == NoteError::Ok, "rename: doi ten");
    check(m.brokenLinks().empty(),                    "rename: khong con link hong");
    check(m.backlinks("zz_test_New").size() == 2,     "rename: A va B tro toi ten moi");
    check(m.searchContent("[[zz_test_New]]").size() == 3,
          "rename: A, B va chinh no deu duoc viet lai");

    // Sua file ngoai chuong trinh, roi doi ten: khong duoc mat noi dung
    {
        std::ofstream out(TEST_DIR / "zz_test_C.md");
        out << "them ngoai chuong trinh [[zz_test_New]]";
    }
    check(m.renameNote("zz_test_New", "zz_test_Newer") == NoteError::Ok, "rename: lan 2");
    check(m.searchContent("them ngoai chuong trinh").size() == 1,
          "rename: noi dung sua ngoai chuong trinh khong bi ghi de");
    check(m.brokenLinks().empty(), "rename: C (sua ngoai) cung duoc cap nhat link");

    cleanupTestFiles();
}



void testLoadReport(){
    cleanupTestFiles();
    const fs::path dir = TEST_DIR / "v2_report"; 
    NoteManager a(dir);
    a.createNote("zz_test_a", "x");
    a.createNote("zz_test_b", "y");

    NoteManager b(dir);
    LoadReport rep = b.loadNotesFromDisk();
    check(rep.ok && rep.loaded == 2 && rep.skipped.empty(),
          "load: bao cao dung 2 note, khong file nao bi bo qua");
    cleanupTestFiles();
}

void testLoadFailureKeepsMemory(){
    cleanupTestFiles();
    const fs::path dir = TEST_DIR / "v2";
    NoteManager m(dir);
    m.createNote("zz_test_keep", "du lieu dang dung duoc");

    // Pha thu muc: thay bang mot FILE cung ten, de viec duyet thu muc that bai
    std::error_code ec;
    fs::remove_all(dir, ec);
    { std::ofstream f(dir); f << "toi la file, khong phai thu muc"; }

    LoadReport rep = m.loadNotesFromDisk();
    check(!rep.ok, "load: bao loi khi khong duyet duoc thu muc");
    check(m.searchTitle("zz_test_keep").size() == 1,
          "load: nap that bai khong xoa du lieu dang co trong RAM");
    cleanupTestFiles();
}




void testCreateErrors(){
    cleanupTestFiles();
    NoteManager m(TEST_DIR);
    check(m.createNote("zz_test_E1", "x") == NoteError::Ok, "create: tao binh thuong");
    check(m.createNote("ZZ_TEST_e1", "x") == NoteError::AlreadyExists, "create: trung ten trong RAM");

    { std::ofstream f(TEST_DIR / "zz_test_E2.md"); f << "tao ngoai chuong trinh"; }
    check(m.createNote("zz_test_E2", "x") == NoteError::AlreadyExists, "create: file da co tren dia");

    check(m.createNote("   ", "x") == NoteError::InvalidTitle, "create: toan dau cach la khong hop le");
    cleanupTestFiles();
}


void testRenameErrors(){
    cleanupTestFiles();
    NoteManager m(TEST_DIR);
    m.createNote("zz_test_R1", "noi dung R1");
    m.createNote("zz_test_R2", "noi dung R2");

    check(m.renameNote("khong_co", "zz_test_X") == NoteError::NotFound,       "rename: note khong ton tai");
    check(m.renameNote("zz_test_R1", "a/b")     == NoteError::InvalidTitle,   "rename: tieu de moi khong hop le");
    check(m.renameNote("zz_test_R1", "zz_test_R2") == NoteError::AlreadyExists, "rename: trung note khac trong RAM");

    // File co tren dia ma RAM chua biet (gia lap tao bang Notepad)
    { std::ofstream f(TEST_DIR / "zz_test_R3.md"); f << "file ngoai quan trong"; }
    check(m.renameNote("zz_test_R1", "zz_test_R3") == NoteError::AlreadyExists, "rename: trung file tren dia");

    std::string s;
    readFileToString(TEST_DIR / "zz_test_R3.md", s);
    check(s == "file ngoai quan trong", "rename: file tren dia KHONG bi ghi de");
    check(m.searchTitle("zz_test_R1").size() == 1, "rename: that bai thi ten cu van con trong RAM");

    check(m.renameNote("zz_test_R1", "ZZ_TEST_R1") == NoteError::Ok, "rename: doi hoa/thuong chinh no van duoc");
    cleanupTestFiles();
}


void testRenameErrors_after(){
    cleanupTestFiles();
    NoteManager m(TEST_DIR);
    m.createNote("zz_test_R1", "noi dung R1");
    m.createNote("zz_test_R2", "noi dung R2");

    check(m.renameNote("khong_co", "zz_test_X") == NoteError::NotFound,       "rename: note khong ton tai");
    check(m.renameNote("zz_test_R1", "a/b")     == NoteError::InvalidTitle,   "rename: tieu de moi khong hop le");
    check(m.renameNote("zz_test_R1", "zz_test_R2") == NoteError::AlreadyExists, "rename: trung note khac trong RAM");

    // File co tren dia ma RAM chua biet (gia lap tao bang Notepad)
    { std::ofstream f(TEST_DIR / "zz_test_R3.md"); f << "file ngoai quan trong"; }
    check(m.renameNote("zz_test_R1", "zz_test_R3") == NoteError::AlreadyExists, "rename: trung file tren dia");

    std::string s;
    readFileToString(TEST_DIR / "zz_test_R3.md", s);
    check(s == "file ngoai quan trong", "rename: file tren dia KHONG bi ghi de");
    check(m.searchTitle("zz_test_R1").size() == 1, "rename: that bai thi ten cu van con trong RAM");

    check(m.renameNote("zz_test_R1", "ZZ_TEST_R1") == NoteError::Ok, "rename: doi hoa/thuong chinh no van duoc");
    cleanupTestFiles();
}



void testEditDeleteErrors(){
    cleanupTestFiles();
    NoteManager m(TEST_DIR);
    m.createNote("zz_test_D1", "cu");

    check(m.editNote("khong_co", "x")     == NoteError::NotFound, "edit: note khong ton tai");
    check(m.editNote("ZZ_TEST_d1", "moi") == NoteError::Ok,       "edit: sua (go khac hoa/thuong)");

    std::string s;
    readFileToString(TEST_DIR / "zz_test_D1.md", s);
    check(s == "moi", "edit: noi dung da duoc ghi xuong dia");

    check(m.deleteNote("khong_co") == NoteError::NotFound, "delete: note khong ton tai");

    // File bi xoa ngoai chuong trinh: van xoa duoc khoi RAM
    std::error_code ec;
    fs::remove(TEST_DIR / "zz_test_D1.md", ec);
    check(m.deleteNote("zz_test_D1") == NoteError::Ok, "delete: file da mat ngoai chuong trinh van xoa khoi RAM");
    check(m.searchTitle("zz_test_D1").empty(),         "delete: RAM sach sau khi xoa");
    cleanupTestFiles();
}




void testOpenNote(){
    cleanupTestFiles();
    NoteManager m(TEST_DIR);
    m.createNote("zz_test_O1", "ban dau");

    OpenResult r = m.openNote("ZZ_TEST_o1");
    check(r.error == NoteError::Ok && r.note.getContent() == "ban dau" && !r.stale,
         "open: mo binh thuong, khong stale");

    // Gia lap Notepad: sua file ngoai chuong trinh
    { std::ofstream f(TEST_DIR / "zz_test_O1.md"); f << "da sua ngoai"; }
    check(m.openNote("zz_test_O1").note.getContent() == "da sua ngoai",
          "open: doc lai tu dia, thay ban sua ngoai");
    check(m.searchContent("da sua ngoai").size() == 1,
          "open: cache RAM cua note do duoc lam moi");

    // File bi xoa ngoai chuong trinh
    std::error_code ec;
    fs::remove(TEST_DIR / "zz_test_O1.md", ec);
    OpenResult r2 = m.openNote("zz_test_O1");
    check(r2.error == NoteError::Ok && r2.stale && r2.note.getContent() == "da sua ngoai",
          "open: file mat -> tra ban RAM va danh dau stale");
    cleanupTestFiles();
}




void testListNotes(){
    cleanupTestFiles();
    NoteManager m(TEST_DIR);
    check(m.listNotes().empty(), "list: rong khi chua co note");

    m.createNote("zz_test_N1", "a");
    m.createNote("zz_test_N2", "b");
    auto list = m.listNotes();
    check(list.size() == 2, "list: co 2 note");
    check(list.size() == 2 && list[0].title == "zz_test_N1" && list[1].title == "zz_test_N2",
          "list: giu thu tu tao");
    check(list.size() == 2 && !list[0].lastModified.empty(), "list: co lastModified");

    m.deleteNote("zz_test_N1");
    auto after = m.listNotes();
    check(after.size() == 1 && after[0].title == "zz_test_N2", "list: phan anh viec xoa");
    cleanupTestFiles();
}