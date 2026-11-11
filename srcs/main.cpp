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

static std::string errorMessage(NoteError e){
    switch(e){
        case NoteError::Ok:            return "Thanh cong";
        case NoteError::NotFound:      return "Khong tim thay ghi chu";
        case NoteError::InvalidTitle:  return "Tieu de khong hop le (rong, chua ky tu cam, hoac ket thuc bang dau cham/dau cach)";
        case NoteError::AlreadyExists: return "Tieu de da ton tai";
        case NoteError::ReadFailed:    return "Khong doc duoc file";
        case NoteError::WriteFailed:   return "Khong ghi duoc file";
        case NoteError::DeleteFailed:  return "Khong xoa duoc file";
    }
    return "Loi khong xac dinh";
}

static void printLoadReport(const LoadReport& r){
    if(!r.ok){
        std::cout << "Khong doc duoc thu muc ghi chu. Giu nguyen du lieu hien co.\n";
        return;
    }
    std::cout << "Da tai " << r.loaded << " ghi chu.\n";
    for(const auto& f : r.skipped)
        std::cout << "  Bo qua (khong doc duoc): " << f << "\n";
}

static bool askNewTitle(const NoteManager& manager, std::string& title){
    while(true){
        title = readLine("Tieu de (de trong de huy): ");
        if(title.empty()) return false;

        NoteError err = manager.validateNewTitle(title);
        if(err == NoteError::Ok) return true;
        std::cout << "  " << errorMessage(err) << ". Moi nhap lai.\n";
    }
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
    printLoadReport(manager.loadNotesFromDisk());

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
                std::string title;
                if(!askNewTitle(manager, title)){ std::cout << "Da huy.\n"; break; }

                std::string content = readContent();          // chi go noi dung khi tieu de da on
                NoteError err = manager.createNote(title, content);
                if(err == NoteError::Ok) std::cout << "Da luu ghi chu.\n";
                else std::cout << "Tao that bai: " << errorMessage(err) << "\n";
                break;
            }
            case 2: {
                auto list = manager.listNotes();
                if(list.empty()) std::cout << "Chua co ghi chu nao.\n";
                for(const auto& n : list){
                    std::cout << "-------------------------------------------\n"
                            << "Title: " << n.title << "\n"
                            << "Last Modified: " << n.lastModified << "\n";
                }
                break;
            }
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
            case 5: {
                OpenResult r = manager.openNote(readLine("Tieu de: "));
                if(r.error != NoteError::Ok){
                    std::cout << "Mo that bai: " << errorMessage(r.error) << "\n";
                    break;
                }
                if(r.stale) std::cout << "(Khong doc lai duoc file, hien ban luu trong bo nho)\n";
                std::cout << "File content:\n" << r.note.getContent() << "\n";
                break;
            }
            case 6: {
                std::string title = readLine("Tieu de can sua: ");
                if(!manager.isTitleExists(title)){ std::cout << "Khong tim thay ghi chu.\n"; break; }

                std::string content = readContent();
                NoteError err = manager.editNote(title, content);
                if(err == NoteError::Ok) std::cout << "Da cap nhat.\n";
                else std::cout << "Sua that bai: " << errorMessage(err) << "\n";
                break;
            }
            case 7: {
                std::string oldT = readLine("Tieu de cu: ");
                std::string newT = readLine("Tieu de moi: ");
                RenameResult r = manager.renameNote(oldT, newT);
                if(r.error != NoteError::Ok){
                    std::cout << "Doi ten that bai: " << errorMessage(r.error) << "\n";
                    break;
                }
                std::cout << "Da doi ten.\n";
                if(r.updatedNotes > 0)
                    std::cout << "Da cap nhat link trong " << r.updatedNotes << " ghi chu.\n";
                for(const auto& t : r.failedNotes)
                    std::cout << "  Khong cap nhat duoc link trong: " << t << "\n";
                break;
            }
            case 8: {
                NoteError err = manager.deleteNote(readLine("Tieu de can xoa: "));
                if(err == NoteError::Ok) std::cout << "Da xoa.\n";
                else std::cout << "Xoa that bai: " << errorMessage(err) << "\n";
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
                printLoadReport(manager.loadNotesFromDisk());
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