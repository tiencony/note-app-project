#include <iostream>
#include <limits>
#include "../headers/NoteManager.h"

static void printMenu(){
    std::cout << "\n===== QUAN LY GHI CHU =====\n"
              << "1. Tao ghi chu\n"
              << "2. Liet ke ghi chu\n"
              << "3. Tim kiem theo tieu de\n"
              << "0. Thoat\n"
              << "Chon: ";
}

int main(){
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
                std::string title, content, line;
                std::cout << "Tieu de: ";
                std::getline(std::cin, title);
                std::cout << "Noi dung (go dong rong de ket thuc):\n";
                while(std::getline(std::cin, line) && !line.empty()){
                    content += line + "\n";
                }
                manager.createNote(title, content);
                break;
            }
            case 2:
                manager.listNotes();
                break;
            case 3: {
                std::string keyword;
                std::cout << "Tu khoa: ";
                std::getline(std::cin, keyword);
                manager.searchTitle(keyword);
                break;
            }
            case 0:
                std::cout << "Tam biet!\n";
                break;
            default:
                std::cout << "Lua chon khong hop le!\n";
        }
    }
    return 0;
}