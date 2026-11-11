// BỘ TEST NÀY DO CLAUDE TẠO RA



#include "test_framework.h"
#include "tests.h"
#include "NoteManager/NoteManager.h"
#include "Utils/Utils.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>
#include <vector>

namespace fs = std::filesystem;

// Thu muc RIENG cho test. Khong bao gio tro toi vault that.
static const fs::path TEST_DIR = "vault_test";

// ======================================================================
//  Tien ich cho test
// ======================================================================

// [DISK] Xoa TOAN BO thu muc test (ke ca thu muc con va file la).
// An toan vi TEST_DIR chi dung cho test; co chot chan phong khi ai do doi nham duong dan.
void cleanupTestFiles(){
    if(TEST_DIR.filename().string() != "vault_test") return;
    std::error_code ec;
    fs::remove_all(TEST_DIR, ec);
}

namespace {

// Dat o dau moi test: don truoc khi chay VA tu don khi ra khoi ham (ke ca khi return som).
// Muon xem file con lai de debug: tam comment dong trong ~TestDir().
struct TestDir {
    TestDir()  { cleanupTestFiles(); }
    ~TestDir() { cleanupTestFiles(); }
};

// [DISK] Gia lap "nguoi dung sua bang Notepad": ghi file ma NoteManager khong hay biet.
void writeExternal(const fs::path& path, const std::string& text){
    std::error_code ec;
    fs::create_directories(path.parent_path(), ec);
    std::ofstream out(path);
    out << text;
}

// [DISK] Doc noi dung file that tren dia (khong qua RAM cua NoteManager).
std::string readDisk(const fs::path& path){
    std::string s;
    readFileToString(path, s);
    return s;
}

// [DISK] Dem so muc (file/thu muc) truc tiep trong thu muc.
int countEntries(const fs::path& dir){
    std::error_code ec;
    int n = 0;
    for(fs::directory_iterator it(dir, ec), end; !ec && it != end; it.increment(ec)) n++;
    return n;
}

} // namespace

// ======================================================================
//  CRUD tong hop
// ======================================================================

void testCrud(){
    TestDir guard;
    NoteManager m(TEST_DIR);

    // --- Tao
    check(m.createNote("zz_test_A", "hoc con tro C++") == NoteError::Ok, "crud: tao A");
    check(m.createNote("zz_test_B", "ke hoach tuan")   == NoteError::Ok, "crud: tao B");
    check(m.createNote("zz_test_C", "ghi chu khac")    == NoteError::Ok, "crud: tao C");
    check(m.listNotes().size() == 3,  "crud: RAM co 3 note");
    check(countEntries(TEST_DIR) == 3, "crud: dia co 3 file");

    // --- Tieu de khong hop le
    check(m.createNote("ZZ_TEST_a", "x") == NoteError::AlreadyExists, "crud: tu choi trung ten khac hoa/thuong");
    check(m.createNote("a/b", "x")       == NoteError::InvalidTitle,  "crud: tu choi ten chua '/'");
    check(m.createNote("", "x")          == NoteError::InvalidTitle,  "crud: tu choi ten rong");
    check(m.createNote("abc.", "x")      == NoteError::InvalidTitle,  "crud: tu choi ten ket thuc bang dau cham");
    check(m.listNotes().size() == 3,   "crud: tao that bai khong lam tang so note");
    check(countEntries(TEST_DIR) == 3, "crud: tao that bai khong sinh them file");

    // --- Tim kiem
    check(m.searchTitle("TEST_A").size() == 1,        "crud: searchTitle khong phan biet hoa/thuong");
    check(m.searchTitle("zz_test_").size() == 3,      "crud: searchTitle ra du 3 note");
    check(m.searchTitle("khong_co").empty(),          "crud: searchTitle khong tim thay");
    check(m.searchContent("CON TRO").size() == 1,     "crud: searchContent tim thay");
    check(m.searchContent("khong_co_tu_nay").empty(), "crud: searchContent khong tim thay");

    // --- Mo / sua
    check(m.openNote("zz_test_a").error == NoteError::Ok,           "crud: mo note (go khac hoa/thuong)");
    check(m.openNote("khong_ton_tai").error == NoteError::NotFound, "crud: mo note khong ton tai");
    check(m.editNote("zz_test_A", "noi dung moi hoan toan") == NoteError::Ok, "crud: sua A");
    check(m.searchContent("hoan toan").size() == 1, "crud: noi dung moi tim duoc");
    check(m.searchContent("con tro").empty(),       "crud: noi dung cu khong con");

    // --- Doi ten
    check(m.renameNote("zz_test_B", "zz_test_B2").error == NoteError::Ok,       "crud: doi ten B -> B2");
    check(m.openNote("zz_test_B").error  == NoteError::NotFound,                "crud: ten cu khong con");
    check(m.openNote("zz_test_B2").error == NoteError::Ok,                      "crud: ten moi mo duoc");
    check(m.renameNote("zz_test_B2", "zz_test_C").error == NoteError::AlreadyExists, "crud: tu choi doi ten trung note khac");
    check(m.renameNote("zz_test_C", "ZZ_TEST_C").error  == NoteError::Ok,       "crud: cho phep doi hoa/thuong chinh no");

    // --- Nap lai tu dia bang doi tuong moi: moi thay doi phai da xuong dia
    {
        NoteManager m2(TEST_DIR);
        LoadReport rep = m2.loadNotesFromDisk();
        check(rep.ok && rep.loaded == 3 && rep.skipped.empty(), "crud: nap lai tu dia: 3 note, khong bo qua file nao");
        check(m2.searchTitle("zz_test_").size() == 3,    "crud: nap lai thay du 3 note");
        check(m2.searchTitle("zz_test_B2").size() == 1,  "crud: ten moi da xuong dia");
        check(m2.searchContent("hoan toan").size() == 1, "crud: noi dung sua da xuong dia");
    }

    // --- Xoa
    check(m.deleteNote("zz_test_A")  == NoteError::Ok,       "crud: xoa A (phan tu dau)");
    check(m.deleteNote("zz_test_C")  == NoteError::Ok,       "crud: xoa C (go khac hoa/thuong)");
    check(m.deleteNote("zz_test_B2") == NoteError::Ok,       "crud: xoa B2 (phan tu cuoi)");
    check(m.deleteNote("zz_test_A")  == NoteError::NotFound, "crud: xoa lai note da xoa -> NotFound");
    check(m.searchTitle("zz_test_").empty(),                 "crud: khong con note nao trong RAM");
    check(countEntries(TEST_DIR) == 0,                       "crud: khong con file nao tren dia");
}

// ======================================================================
//  Backlinks / link hong / chi muc link
// ======================================================================

void testBacklinks(){
    TestDir guard;
    NoteManager b(TEST_DIR);
    b.createNote("zz_test_L1", "xem [[zz_test_L2]]");
    b.createNote("zz_test_L2", "khong co link nao");
    b.createNote("zz_test_L3", "[[ZZ_TEST_L2]] va [[zz_test_L2]]");
    b.createNote("zz_test_L4", "[[zz_test_L4]] tu tro vao minh");

    check(b.backlinks("zz_test_L2") == std::vector<std::string>{"zz_test_L1", "zz_test_L3"},
          "backlinks: L1 va L3 tro toi L2 (L3 chi tinh 1 lan), thu tu on dinh");
    check(b.backlinks("zz_test_l2").size() == 2,
          "backlinks: khong phan biet hoa/thuong");
    check(b.backlinks("zz_test_L1").empty(),
          "backlinks: khong ai tro toi L1");
    check(b.backlinks("zz_test_L4").empty(),
          "backlinks: link tro vao chinh no khong tinh");
    check(b.backlinks("khong_ton_tai").empty(),
          "backlinks: note khong ton tai thi rong");
}

void testBrokenLinks(){
    TestDir guard;
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
}

void testBrokenLinksDedup(){
    TestDir guard;
    NoteManager b(TEST_DIR);
    b.createNote("zz_test_D1", "[[zz_test_ghost]] va [[ZZ_TEST_GHOST]] va [[zz_test_ghost.md]]");

    check(b.brokenLinks().size() == 1,
          "brokenLinks: cung mot dich (khac hoa/thuong, co/khong .md) chi bao 1 lan");
}

void testLinkIndexFresh(){
    TestDir guard;
    NoteManager b(TEST_DIR);

    // --- Chi muc tu lam moi khi du lieu doi qua NoteManager
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

    // --- Sua NGOAI chuong trinh: chi muc chi moi sau khi tai lai tu dia
    b.createNote("zz_test_X", "[[zz_test_Y]]");
    b.createNote("zz_test_Y", "y");
    check(b.backlinks("zz_test_Y").size() == 1, "index: Y co 1 backlink tu X");

    writeExternal(TEST_DIR / "zz_test_X.md", "khong con link nua");
    // [HANH VI DA BIET, TAM CHAP NHAN] RAM chua biet ban sua ngoai. Khi sau nay lam
    // syncWithDisk thi dong kiem tra nay se doi thanh .empty().
    check(b.backlinks("zz_test_Y").size() == 1, "index: chua tai lai thi RAM van cu (hanh vi da biet)");

    b.loadNotesFromDisk();
    check(b.backlinks("zz_test_Y").empty(), "index: tai lai tu dia -> chi muc duoc tinh lai");
}

// ======================================================================
//  Nap tu dia (LoadReport)
// ======================================================================

void testLoadReport(){
    TestDir guard;
    const fs::path dir = TEST_DIR / "report";
    {
        NoteManager a(dir);
        a.createNote("zz_test_a", "x");
        a.createNote("zz_test_b", "y");
    }
    writeExternal(dir / "ghi_chu.txt", "khong phai note");

    NoteManager b(dir);
    LoadReport rep = b.loadNotesFromDisk();
    check(rep.ok && rep.loaded == 2 && rep.skipped.empty(),
          "load: bao cao dung 2 note, khong file nao bi bo qua");
    check(b.searchTitle("ghi_chu").empty(),
          "load: file khong phai .md khong duoc nap");

    // Goi load hai lan khong bi nhan doi
    rep = b.loadNotesFromDisk();
    check(rep.ok && rep.loaded == 2 && b.searchTitle("zz_test_").size() == 2,
          "load: tai lai khong nhan doi note");
}

void testLoadFailureKeepsMemory(){
    TestDir guard;
    const fs::path dir = TEST_DIR / "broken";
    NoteManager m(dir);
    m.createNote("zz_test_keep", "du lieu dang dung duoc");

    // Pha thu muc: thay bang mot FILE cung ten, de viec duyet thu muc that bai
    std::error_code ec;
    fs::remove_all(dir, ec);
    writeExternal(dir, "toi la file, khong phai thu muc");

    LoadReport rep = m.loadNotesFromDisk();
    check(!rep.ok, "load: bao loi khi khong duyet duoc thu muc");
    check(m.searchTitle("zz_test_keep").size() == 1,
          "load: nap that bai khong xoa du lieu dang co trong RAM");
}

// ======================================================================
//  Tao / kiem tra tieu de
// ======================================================================

void testCreateErrors(){
    TestDir guard;
    NoteManager m(TEST_DIR);
    check(m.createNote("zz_test_E1", "x") == NoteError::Ok,            "create: tao binh thuong");
    check(m.createNote("ZZ_TEST_e1", "x") == NoteError::AlreadyExists, "create: trung ten trong RAM");

    // File co san tren dia ma RAM chua biet: khong duoc ghi de
    writeExternal(TEST_DIR / "zz_test_E2.md", "tao ngoai chuong trinh");
    check(m.createNote("zz_test_E2", "x") == NoteError::AlreadyExists, "create: file da co tren dia");
    check(readDisk(TEST_DIR / "zz_test_E2.md") == "tao ngoai chuong trinh",
          "create: file tren dia KHONG bi ghi de");
    check(m.searchTitle("zz_test_E2").empty(), "create: that bai thi khong them vao RAM");

    // Cac luat tieu de
    check(m.createNote("   ", "x")            == NoteError::InvalidTitle, "create: toan dau cach");
    check(m.createNote(" zz_test_E3", "x")    == NoteError::InvalidTitle, "create: bat dau bang dau cach");
    check(m.createNote("zz_test_E4 ", "x")    == NoteError::InvalidTitle, "create: ket thuc bang dau cach");
    check(m.createNote("zz_test_E5.md", "x")  == NoteError::InvalidTitle, "create: ket thuc bang .md");
}

void testValidateNewTitle(){
    TestDir guard;
    NoteManager m(TEST_DIR);

    check(m.validateNewTitle("zz_test_V")   == NoteError::Ok,           "validate: ten moi hop le");
    check(m.validateNewTitle("a/b")         == NoteError::InvalidTitle, "validate: ky tu cam");
    check(m.validateNewTitle("abc.md")      == NoteError::InvalidTitle, "validate: duoi .md");
    check(m.validateNewTitle(" abc")        == NoteError::InvalidTitle, "validate: bat dau bang dau cach");

    m.createNote("zz_test_V", "x");
    check(m.validateNewTitle("ZZ_TEST_v")   == NoteError::AlreadyExists, "validate: trung ten trong RAM");

    writeExternal(TEST_DIR / "zz_test_V2.md", "ngoai");
    check(m.validateNewTitle("zz_test_V2")  == NoteError::AlreadyExists, "validate: file co san tren dia");

    check(m.listNotes().size() == 1,   "validate: khong tao them note nao (khong co tac dung phu)");
    check(countEntries(TEST_DIR) == 2, "validate: khong tao them file nao");
}

// ======================================================================
//  Sua / xoa / mo / liet ke
// ======================================================================

void testEditDeleteErrors(){
    TestDir guard;
    NoteManager m(TEST_DIR);
    m.createNote("zz_test_D1", "cu");

    check(m.editNote("khong_co", "x")     == NoteError::NotFound, "edit: note khong ton tai");
    check(m.editNote("ZZ_TEST_d1", "moi") == NoteError::Ok,       "edit: sua (go khac hoa/thuong)");
    check(readDisk(TEST_DIR / "zz_test_D1.md") == "moi",          "edit: noi dung da duoc ghi xuong dia");
    check(countEntries(TEST_DIR) == 1,                            "edit: khong con file tam sau khi ghi");

    check(m.deleteNote("khong_co") == NoteError::NotFound, "delete: note khong ton tai");

    // File bi xoa ngoai chuong trinh: van xoa duoc khoi RAM
    std::error_code ec;
    fs::remove(TEST_DIR / "zz_test_D1.md", ec);
    check(m.deleteNote("zz_test_D1") == NoteError::Ok, "delete: file da mat ngoai chuong trinh van xoa khoi RAM");
    check(m.searchTitle("zz_test_D1").empty(),         "delete: RAM sach sau khi xoa");
}

void testOpenNote(){
    TestDir guard;
    NoteManager m(TEST_DIR);
    m.createNote("zz_test_O1", "ban dau");

    OpenResult r = m.openNote("ZZ_TEST_o1");
    check(r.error == NoteError::Ok && r.note.getContent() == "ban dau" && !r.stale,
          "open: mo binh thuong, khong stale");
    check(m.openNote("ZZ_TEST_O1.md").error == NoteError::Ok,
          "open: go kem duoi .md van tim thay (ten duoc chuan hoa)");

    // Gia lap Notepad: sua file ngoai chuong trinh
    writeExternal(TEST_DIR / "zz_test_O1.md", "da sua ngoai");
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
}

void testListNotes(){
    TestDir guard;
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
}

// ======================================================================
//  Doi ten (RenameResult)
// ======================================================================

void testRenameErrors(){
    TestDir guard;
    NoteManager m(TEST_DIR);
    m.createNote("zz_test_R1", "noi dung R1");
    m.createNote("zz_test_R2", "noi dung R2");

    check(m.renameNote("khong_co", "zz_test_X").error    == NoteError::NotFound,      "rename: note khong ton tai");
    check(m.renameNote("zz_test_R1", "a/b").error        == NoteError::InvalidTitle,  "rename: ky tu cam");
    check(m.renameNote("zz_test_R1", "abc.md").error     == NoteError::InvalidTitle,  "rename: duoi .md");
    check(m.renameNote("zz_test_R1", "zz_test_R2").error == NoteError::AlreadyExists, "rename: trung note khac trong RAM");

    // File co tren dia ma RAM chua biet (gia lap tao bang Notepad): khong duoc ghi de
    writeExternal(TEST_DIR / "zz_test_R3.md", "file ngoai quan trong");
    check(m.renameNote("zz_test_R1", "zz_test_R3").error == NoteError::AlreadyExists, "rename: trung file tren dia");
    check(readDisk(TEST_DIR / "zz_test_R3.md") == "file ngoai quan trong",
          "rename: file tren dia KHONG bi ghi de");
    check(m.openNote("zz_test_R1").error == NoteError::Ok,
          "rename: that bai thi ten cu van con");
    check(readDisk(TEST_DIR / "zz_test_R1.md") == "noi dung R1",
          "rename: that bai thi file cu van nguyen");

    // Chi doi hoa/thuong chinh no thi duoc phep
    check(m.renameNote("zz_test_R1", "ZZ_TEST_R1").error == NoteError::Ok,
          "rename: doi hoa/thuong chinh no van duoc");
    check(m.listNotes()[0].title == "ZZ_TEST_R1", "rename: title trong RAM da doi theo");
}

void testRenameReport(){
    TestDir guard;
    NoteManager m(TEST_DIR);
    m.createNote("zz_test_old", "toi");
    m.createNote("zz_test_P1",  "[[zz_test_old]]");
    m.createNote("zz_test_P2",  "[[zz_test_old]] va [[ZZ_TEST_OLD]]");
    m.createNote("zz_test_Q",   "khong lien quan");

    RenameResult r = m.renameNote("zz_test_old", "zz_test_new");
    check(r.error == NoteError::Ok && r.updatedNotes == 2 && r.failedNotes.empty(),
          "rename report: bao dung 2 note duoc cap nhat link");
    check(readDisk(TEST_DIR / "zz_test_P1.md") == "[[zz_test_new]]",
          "rename report: link da duoc ghi xuong dia");
    check(readDisk(TEST_DIR / "zz_test_Q.md") == "khong lien quan",
          "rename report: note khong lien quan khong bi dong toi");

    RenameResult r2 = m.renameNote("zz_test_Q", "zz_test_Q2");
    check(r2.error == NoteError::Ok && r2.updatedNotes == 0 && r2.failedNotes.empty(),
          "rename report: khong ai link toi -> 0 note duoc cap nhat");

    // lastModified phai la thoi gian that cua file (doi ten khong lam doi mtime)
    std::string ramTime;
    for(const auto& n : m.listNotes()) if(n.title == "zz_test_Q2") ramTime = n.lastModified;
    check(!ramTime.empty() && ramTime == getFileTime(TEST_DIR / "zz_test_Q2.md"),
          "rename report: lastModified khop thoi gian that cua file");
}

void testRenameUpdatesLinks(){
    TestDir guard;
    NoteManager m(TEST_DIR);
    m.createNote("zz_test_Old", "tu tro [[zz_test_Old]]");
    m.createNote("zz_test_A", "xem [[zz_test_Old]]");
    m.createNote("zz_test_B", "xem [[ ZZ_TEST_OLD.md ]]");
    m.createNote("zz_test_C", "khong lien quan");

    RenameResult r = m.renameNote("zz_test_Old", "zz_test_New");
    check(r.error == NoteError::Ok && r.updatedNotes == 3,
          "rename links: doi ten, 3 note duoc cap nhat (A, B va chinh no)");
    check(m.brokenLinks().empty(),                "rename links: khong con link hong");
    check(m.backlinks("zz_test_New").size() == 2, "rename links: A va B tro toi ten moi");
    check(m.searchContent("[[zz_test_New]]").size() == 3,
          "rename links: A, B va chinh no deu duoc viet lai");

    // Sua file NGOAI chuong trinh, roi doi ten: khong duoc mat noi dung
    writeExternal(TEST_DIR / "zz_test_C.md", "them ngoai chuong trinh [[zz_test_New]]");
    r = m.renameNote("zz_test_New", "zz_test_Newer");
    check(r.error == NoteError::Ok, "rename links: doi ten lan 2");
    check(m.searchContent("them ngoai chuong trinh").size() == 1,
          "rename links: noi dung sua ngoai chuong trinh khong bi ghi de");
    check(m.brokenLinks().empty(),
          "rename links: C (sua ngoai) cung duoc cap nhat link");
    check(readDisk(TEST_DIR / "zz_test_C.md") == "them ngoai chuong trinh [[zz_test_Newer]]",
          "rename links: file C tren dia da duoc cap nhat dung");
}

void testRenameSelfLink(){
    TestDir guard;
    NoteManager m(TEST_DIR);
    m.createNote("zz_test_S", "toi tu tro [[zz_test_S]]");

    RenameResult r = m.renameNote("zz_test_S", "zz_test_S2");
    check(r.error == NoteError::Ok, "rename self: doi ten thanh cong");
    check(m.searchContent("[[zz_test_S2]]").size() == 1,
          "rename self: link tro vao chinh no cung duoc cap nhat");
    check(m.brokenLinks().empty(), "rename self: khong tao link hong");
}

void testRenameRefreshesCache(){
    TestDir guard;
    NoteManager m(TEST_DIR);
    m.createNote("zz_test_old2", "a");
    m.createNote("zz_test_R", "noi dung cu");

    writeExternal(TEST_DIR / "zz_test_R.md", "noi dung moi #ext2");
    // [HANH VI DA BIET, TAM CHAP NHAN] RAM chua biet ban sua ngoai
    check(m.searchContent("#ext2").empty(), "refresh: RAM chua biet ban sua ngoai (hanh vi da biet)");

    m.renameNote("zz_test_old2", "zz_test_new2");
    check(m.searchContent("#ext2").size() == 1,
          "refresh: sau rename, cache cua note khong lien quan duoc lam moi tu dia");
}