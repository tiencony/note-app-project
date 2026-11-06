#include "../headers/NoteManager.h"
#include "../headers/Utils.h"

#include <fstream>
#include <filesystem>
#include <iostream>
#include <algorithm>
#include <cctype>

NoteManager::NoteManager(int initialCapacity){
    capacity = (initialCapacity > 0) ? initialCapacity : 10;
    count = 0;
    notesArray = new Note[capacity];
}

NoteManager::~NoteManager(){
    delete[] notesArray;
}

void NoteManager::resize(){
    int newCapacity = capacity * 2;
    Note* temp = new Note[newCapacity];
    for(int i = 0; i < count; i++){
        temp[i] = notesArray[i];
    }

    delete[] notesArray;
    notesArray = temp;
    capacity = newCapacity;
}


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

    outFile << "# " << title << "\n\n";
    outFile << content << "\n";
    outFile.close();
    std::cout << "Da luu ghi chu thanh cong vao " << filePath << std::endl;

    if(count >= capacity){
        resize();
    }

    notesArray[count] = Note(title, content, "Today");
    count++;
    return true;

}    


void NoteManager::listNotes() const{
    if(count == 0) return;
    for(int i = 0; i < count; i++){
        notesArray[i].displayInfo();
    }
}


void NoteManager::searchTitle(const std::string& keyword) const{
    bool found = false;
    for(int i = 0; i < count; i++){
        if(containsIgnoreCase(notesArray[i].getTitle(), keyword)){
            notesArray[i].displayInfo();
            found = true;
        }
    }
    if(!found){
        std::cout << "Khong tim duoc file chua tu khoa: " << keyword << std::endl;
    }
}



bool NoteManager::checkTitle(const std::string& input) const{
    if(input.empty() || input.find_first_not_of(' ') == std::string::npos){
        std::cout << "Tieu de khong duoc de trong!\n";
        return false;
    }
    if(input.find_first_of("/\\:*?\"<>|") != std::string::npos){
        std::cout << "Tieu de chua ky tu khong hop le!\n";
        return false;
    }
    return true;
}


bool NoteManager::isTitleExists(const std::string& title) const{
    for(int i = 0; i < count; i++){
        if(notesArray[i].getTitle() == title){
            return true;
        }
    }
    return false;
}