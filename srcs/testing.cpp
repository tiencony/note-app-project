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



bool NoteManager::createNote(const std::string& title, const std::string& content){
    if(!checkTitle(title)) return false;
    if(isTitleExists(title)){
        std::cerr << "Loi: Tieu de da ton tai\n";
        return false;
    }
    std::string dir = "vault/notes";
    std::filesystem::create_directories(dir);

    std::string filePath = dir + "/" + title + ".md";
    std::ofstream outFile(filePath);

    if(!outFile){
        std::cerr << "Loi: Khong the tao file ghi chu tai " << filePath << std::endl;
        return false;
    }

    outFile << content;
    outFile.close();
    std::cout << "Da luu ghi chu thanh cong vao " << filePath << std::endl;

    if(count >= capacity){
        resize();
    }

    notesArray[count] = Note(title, content, getCurrentTime());
    count++;
    return true;

}    