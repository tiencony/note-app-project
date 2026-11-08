#include <iostream>
#include <limits>
#include <string>
#include <set>
#include "../headers/NoteManager.h"
#include "../headers/Utils.h"
#include <filesystem>

static const std::string NOTES_DIR = "vault/notes";
namespace fs = std::filesystem;
// ---------- Tien ich nhap lieu ----------
static void cleanupTestFiles(){
    std::error_code ec;
    std::vector<fs::path> toDelete;
    for(const auto& e : fs::directory_iterator(NOTES_DIR, ec)){
        if(e.path().filename().string().rfind("zz_test", 0) == 0){
            toDelete.push_back(e.path());
        }
    }
    for(const auto& p : toDelete) fs::remove(p, ec);

}


static std::string readLine(const std::string& prompt){
    std::cout << prompt;
    std::string s;
    std::getline(std::cin, s);
    return s;
}

static std::string readContent(){
    std::cout << "Noi dung (go :done tren mot dong rieng de ket thuc):\n";
    std::string content, line;
    while(std::getline(std::cin, line) && line != ":done"){
        content += line + "\n";
    }
    return content;
}


// ---------- Test tu dong ----------
static int failed = 0;

static void check(bool condition, const std::string& name){
    std::cout << (condition ? "[PASS] " : "[FAIL] ") << name << "\n";
    if(!condition) failed++;
}

static void testTags(){
    
    using Tags = std::set<std::string>;

    check(extractTags("hoc #cpp va #Very-important")
              == Tags{"cpp", "very-important"},
          "extractTags: lay duoc 2 tag, doi chu thuong");

    check(extractTags("# Tieu de\nnoi dung").empty(),
          "extractTags: heading '# ' khong phai tag");

    check(extractTags("abc#def").empty(),
          "extractTags: '#' dinh lien chu thi khong phai tag");

    check(extractTags("#start\nxuong dong\n#nl")
              == Tags{"start", "nl"},
          "extractTags: tag o dau chuoi va sau xuong dong");

    check(extractTags("#cpp #CPP #Cpp").size() == 1,
          "extractTags: trung tag chi tinh 1");
    check(extractTags("xem (#cpp) nhe") == Tags{"cpp"}, "extractTags: lay duoc 1 tag");
    check(normalizeTag("#CPP") == "cpp", "normalizeTag: bo # va doi chu thuong");
    check(normalizeTag("Cpp") == "cpp",  "normalizeTag: khong co # van dung");

    check(extractTags("xem (#cpp) nhe") == Tags{"cpp"},
        "extractTags: tag trong ngoac don");
    check(extractTags("noi \"#cpp\" nhe") == Tags{"cpp"},
        "extractTags: tag trong ngoac kep");
    check(extractTags("http://a.com/page#section").empty(),
        "extractTags: neo URL khong phai tag");
    check(extractTags("[[Note#Heading]]").empty(),
        "extractTags: heading trong link khong phai tag");
    }

static void testLinks(){
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

static void testBacklinks(){
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

    // don dep
    // b.deleteNote("zz_test_L1");
    // b.deleteNote("zz_test_L2");
    // b.deleteNote("zz_test_L3");
    // b.deleteNote("zz_test_L4");
}


static int runAutoTest(){
    cleanupTestFiles();
    std::cout << "=== TEST TU DONG (tao file zz_test_* trong vault/notes, cuoi se xoa) ===\n";
    testLinks();
    testTags();
    NoteManager m;   // capacity = 2 de test resize o note thu 3

    // Tao + resize
    check(m.createNote("zz_test_A", "hoc con tro C++"), "Tao note A");
    check(m.createNote("zz_test_B", "ke hoach tuan"),   "Tao note B");
    check(m.createNote("zz_test_C", "ghi chu khac"),    "Tao note C (kich hoat resize)");

    // Kiem tra tieu de
    check(!m.createNote("ZZ_TEST_a", "x"), "Tu choi trung ten khac hoa/thuong");
    check(!m.createNote("a/b", "x"),       "Tu choi ten chua '/'");
    check(!m.createNote("", "x"),          "Tu choi ten rong");
    check(!m.createNote("abc.", "x"),      "Tu choi ten ket thuc bang dau cham");

    // Tim kiem
    check(m.searchTitle("TEST_A").size() == 1,   "searchTitle khong phan biet hoa/thuong");
    check(m.searchTitle("zz_test_").size() == 3, "searchTitle ra du 3 note");
    check(m.searchTitle("khong_co").empty(),     "searchTitle khong tim thay");
    check(m.searchContent("CON TRO").size() == 1, "searchContent tim thay");
    check(m.searchContent("khong_co_tu_nay").empty(), "searchContent khong tim thay");

    // Mo / sua
    check(m.openNote("zz_test_a"), "Mo note (go khac hoa/thuong)");
    check(!m.openNote("khong_ton_tai"), "Mo note khong ton tai tra ve false");
    check(m.editNote("zz_test_A", "noi dung moi hoan toan"), "Sua note A");
    check(m.searchContent("hoan toan").size() == 1, "Noi dung moi tim duoc");
    check(m.searchContent("con tro").empty(),       "Noi dung cu khong con");

    // Doi ten
    check(m.renameNote("zz_test_B", "zz_test_B2"), "Doi ten B -> B2");
    check(!m.openNote("zz_test_B"),                "Ten cu khong con");
    check(m.openNote("zz_test_B2"),                "Ten moi mo duoc");
    check(!m.renameNote("zz_test_B2", "zz_test_C"), "Tu choi doi ten trung note khac");
    check(m.renameNote("zz_test_C", "ZZ_TEST_C"),   "Cho phep doi hoa/thuong chinh no");

    // Doc lai tu o cung bang doi tuong moi
    NoteManager m2;
    m2.loadNotesFromDisk();
    check(m2.searchTitle("zz_test_").size() == 3, "Load lai tu dia thay du 3 note");
    check(m2.searchContent("hoan toan").size() == 1, "Noi dung sua da duoc luu xuong dia");

    // Xoa (don dep luon)
    check(m.deleteNote("zz_test_A"),  "Xoa A (phan tu dau)");
    check(m.deleteNote("zz_test_C"),  "Xoa C");
    check(m.deleteNote("zz_test_B2"), "Xoa B2 (phan tu cuoi)");
    check(!m.deleteNote("zz_test_A"), "Xoa lai note da xoa tra ve false");
    check(m.searchTitle("zz_test_").empty(), "Khong con note nao");
    
    // Test backlinks
    testBacklinks();
    cleanupTestFiles();
    std::cout << "\nKet qua: " << (failed == 0 ? "TAT CA DEU PASS" : "CO TEST THAT BAI")
              << " (" << failed << " that bai)\n";
    return failed == 0 ? 0 : 1;
    
    
}

// ---------- Menu tuong tac ----------
static void printMenu(){
    std::cout << "\n===== QUAN LY GHI CHU =====\n"
              << "1. Tao ghi chu\n"
              << "2. Liet ke ghi chu\n"
              << "3. Tim theo tieu de\n"
              << "4. Tim theo noi dung\n"
              << "5. Mo ghi chu\n"
              << "6. Sua noi dung ghi chu\n"
              << "7. Doi ten ghi chu\n"
              << "8. Xoa ghi chu\n"
              << "9. Hien cac danh sach theo Tag\n"
              << "10. Liet ke cac tag\n"
              << "11. Xem backlinks cua ghi chu\n"
              << "0. Thoat\n"
              << "Chon: ";
}

static void runMenu(){
    NoteManager manager;
    manager.loadNotesFromDisk();

    int choice = -1;
    while(choice != 0){
        printMenu();
        if(!(std::cin >> choice)){
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Vui long nhap so!\n";
            choice = -1;
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        switch(choice){
            case 1: {
                std::string title = readLine("Tieu de: ");
                std::string content = readContent();
                manager.createNote(title, content);
                break;
            }
            case 2:
                manager.listNotes();
                break;
            case 3: {
                auto results = manager.searchTitle(readLine("Tu khoa: "));
                if(results.empty()) std::cout << "Khong tim thay.\n";
                for(const auto& t : results) std::cout << "- " << t << "\n";
                break;
            }
            case 4: {
                auto results = manager.searchContent(readLine("Tu khoa: "));
                if(results.empty()) std::cout << "Khong tim thay.\n";
                for(const auto& r : results)
                    std::cout << "- " << r.title << ": " << r.snippet << "\n";
                break;
            }
            case 5:
                if(!manager.openNote(readLine("Tieu de: ")))
                    std::cout << "Khong tim thay ghi chu.\n";
                break;
            case 6: {
                std::string title = readLine("Tieu de can sua: ");
                std::string content = readContent();
                if(manager.editNote(title, content)) std::cout << "Da cap nhat.\n";
                else std::cout << "Sua that bai.\n";
                break;
            }
            case 7: {
                std::string oldT = readLine("Tieu de cu: ");
                std::string newT = readLine("Tieu de moi: ");
                if(manager.renameNote(oldT, newT)) std::cout << "Da doi ten.\n";
                else std::cout << "Doi ten that bai.\n";
                break;
            }
            case 8:{
                if(manager.deleteNote(readLine("Tieu de can xoa: "))) std::cout << "Da xoa.\n";
                else std::cout << "Xoa that bai.\n";
                break;
            }
            case 9: {
                auto results = manager.listByTag(readLine("Tag: "));
                if(results.empty()) std::cout << "Khong co ghi chu nao.\n";
                for(const auto& t : results) std::cout << "- " << t << "\n";
                break;
            }

            case 10: {
                auto counts = manager.tagCounts();
                if(counts.empty()) std::cout << "Chua co tag nao.\n";
                for(const auto& item : counts){
                    std::cout << "#" << item.first << " (" << item.second << ")\n";
                }
                break;
            }
            case 11: {
                auto results = manager.backlinks(readLine("Tieu de: "));
                if(results.empty()) std::cout << "Khong co ghi chu nao tro toi.\n";
                for(const auto& t : results) std::cout << "- " << t << "\n";
                break;
            }       
            case 0:
                std::cout << "Tam biet!\n";
                break;
            default:
                std::cout << "Lua chon khong hop le!\n";
        }
    }
}

int main(int argc, char* argv[]){
    if(argc > 1 && std::string(argv[1]) == "--test"){
        return runAutoTest();
    }
    runMenu();
    return 0;
}