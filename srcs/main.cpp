#include <iostream>
#include <limits>
#include <string>
#include "NoteManager/NoteManager.h"

// ---------- Tien ich nhap lieu ----------
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

// ---------- Menu ----------
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
              << "9. Hien cac ghi chu theo Tag\n"
              << "10. Liet ke cac tag\n"
              << "11. Xem backlinks cua ghi chu\n"
              << "12. Liet ke link hong\n"
              << "13. Tai lai tu dia\n"
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
            case 8:
                if(manager.deleteNote(readLine("Tieu de can xoa: "))) std::cout << "Da xoa.\n";
                else std::cout << "Xoa that bai.\n";
                break;
            case 9: {
                auto results = manager.listByTag(readLine("Tag: "));
                if(results.empty()) std::cout << "Khong co ghi chu nao.\n";
                for(const auto& t : results) std::cout << "- " << t << "\n";
                break;
            }
            case 10: {
                auto counts = manager.tagCounts();
                if(counts.empty()) std::cout << "Chua co tag nao.\n";
                for(const auto& item : counts)
                    std::cout << "#" << item.first << " (" << item.second << ")\n";
                break;
            }
            case 11: {
                auto results = manager.backlinks(readLine("Tieu de: "));
                if(results.empty()) std::cout << "Khong co ghi chu nao tro toi.\n";
                for(const auto& t : results) std::cout << "- " << t << "\n";
                break;
            }
            case 12: {
                auto broken = manager.brokenLinks();
                if(broken.empty()) std::cout << "Khong co link hong.\n";
                for(const auto& b : broken)
                    std::cout << "- " << b.from << " -> [[" << b.target << "]]\n";
                break;
            }
            case 13:
                manager.loadNotesFromDisk();
                break;
            case 0:
                std::cout << "Tam biet!\n";
                break;
            default:
                std::cout << "Lua chon khong hop le!\n";
        }
    }
}

int main(){
    runMenu();
    return 0;
}