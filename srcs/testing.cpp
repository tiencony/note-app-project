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

