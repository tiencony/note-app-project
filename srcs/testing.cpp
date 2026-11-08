#include "../headers/NoteManager.h"
#include "../headers/Utils.h"
#include "../headers/Note.h"

#include <fstream>
#include <filesystem>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <sstream>

namespace fs = std::filesystem;



bool NoteManager::renameNote(const std::string& oldTitle, const std::string& newTitle){
    int i = findIndex(oldTitle);
    if(i == -1) return false;
    if(!checkTitle(newTitle)) return false;

    // Cho phep doi chu cai hoa/thuong voi chinh no
    int j = findIndex(newTitle);
    if(j != -1 && j != i){
        std::cerr << "Loi: Tieu de moi da ton tai\n";
        return false;
    }

    fs::rename(makePath(notesArray[i].getTitle()), makePath(newTitle));
    notesArray[i].setContent(newTitle);
    notesArray[i].setLastModified(getCurrentTime());
    return true;
}


// sua lai them tinh nang tu dong doc tu o dia len RAM moi lan goi
bool NoteManager::openNote(const std::string& title){
    int i = findIndex(title);
    if(i == -1) return false;

    reloadNote(i);
    notesArray[i].displayNote();
    return true;
}

bool NoteManager::reloadNote(int i){
    std::string path = makePath(notesArray[i].getTitle());
    std::string content;
    if(!readFileToString(path, content)){
        std::cerr << "Khong the doc file: " << path << std::endl;
        return false;
    }
    notesArray[i].setContent(content);
    notesArray[i].setLastModified(getFileTime(path));
    return true;

}



void NoteManager::loadNotesFromDisk(){
    fs::create_directories(NOTES_DIR);
    count = 0;

    for (const auto &entry : fs::directory_iterator(NOTES_DIR)){
        if (!entry.is_regular_file() || entry.path().extension() != ".md") continue;

        std::string content;
        if(!readFileToString(entry.path(), content)) continue;
        
        if(count >= capacity) resize();
        notesArray[count] = Note(entry.path().stem().string(), content, getFileTime(entry.path()));
        count++;
    }
    std::cout << "Da tai " << count << " ghi chu.\n";
}
