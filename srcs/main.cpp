#include <iostream>
#include <limits>
#include <string>
#include "../headers/NoteManager.h"

// ---------- Tien ich nhap lieu ----------
static std::string readLine(const std::string& prompt){
    std::cout << prompt;
    std::string s;
    std::getline(std::cin, s);
    return s;
}

static std::string readContent(){
    std::cout << "Noi dung (go dong rong de ket thuc):\n";
    std::string content, line;
    while(std::getline(std::cin, line) && !line.empty()){
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

static int runAutoTest(){
    std::cout << "=== TEST TU DONG (tao file zz_test_* trong vault/notes, cuoi se xoa) ===\n";

    NoteManager m(2);   // capacity = 2 de test resize o note thu 3

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
    NoteManager m2(2);
    m2.loadNotesFromDisk();
    check(m2.searchTitle("zz_test_").size() == 3, "Load lai tu dia thay du 3 note");
    check(m2.searchContent("hoan toan").size() == 1, "Noi dung sua da duoc luu xuong dia");

    // Xoa (don dep luon)
    check(m.deleteNote("zz_test_A"),  "Xoa A (phan tu dau)");
    check(m.deleteNote("zz_test_C"),  "Xoa C");
    check(m.deleteNote("zz_test_B2"), "Xoa B2 (phan tu cuoi)");
    check(!m.deleteNote("zz_test_A"), "Xoa lai note da xoa tra ve false");
    check(m.searchTitle("zz_test_").empty(), "Khong con note nao");

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
              << "0. Thoat\n"
              << "Chon: ";
}

static void runMenu(){
    NoteManager manager(2);
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