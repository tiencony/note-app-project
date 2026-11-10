#include "NoteManager/NoteManager.h"
#include "Utils/FileUtils.h"     // readFileToString
#include "Utils/StringUtils.h"   // equalsIgnoreCase, normalizeTitle
#include "Utils/TimeUtils.h"     // getCurrentTime, getFileTime
#include "Utils/Parser.h"
#include "Utils/Debug.h"


#include <filesystem>
#include <fstream>
#include <iostream>
#include <system_error>

namespace fs = std::filesystem;
static const std::string NOTES_DIR = "vault/notes";  

std::string NoteManager::makePath(const std::string &title) const{
    return NOTES_DIR + "/" + title + ".md";
}




bool NoteManager::reloadNote(int i){
    std::string path = makePath(notesArray[i].getTitle());
    std::string content;
    if(!readFileToString(path, content)){
        std::cerr << "Khong the doc file: " << path << std::endl;
        return false;
    }
    if(notesArray[i].getContent() != content) linkIndexDirty = true;
    notesArray[i].setContent(content);
    notesArray[i].setLastModified(getFileTime(path));
    return true;

}







bool NoteManager::createNote(const std::string &title, const std::string &content){
    if (!checkTitle(title)) return false;
    if (isTitleExists(title)){
        std::cerr << "Loi: Tieu de da ton tai\n";
        return false;
    }

    fs::create_directories(NOTES_DIR);
    std::string filePath = makePath(title);

    std::error_code ec;
    if (fs::exists(filePath, ec)) {
        std::cerr << "Loi: File da ton tai tren dia (co the ban vua tao ngoai chuong trinh)\n";
        return false;
    }
    std::ofstream outFile(filePath);

    if (!outFile){
        std::cerr << "Loi: Khong the tao file ghi chu tai " << filePath << std::endl;
        return false;
    }

    outFile << content;
    outFile.close();
    std::cout << "Da luu ghi chu thanh cong vao " << filePath << std::endl;



    notesArray.push_back(Note(title, content, getCurrentTime()));
    linkIndexDirty = true;
    return true;
}

void NoteManager::listNotes() const{
    if (notesArray.empty()) return;
    for(const Note& note : notesArray){
        note.displayInfo();
    }
}


bool NoteManager::checkTitle(const std::string &input) const{
    if (input.empty() || input.find_first_not_of(' ') == std::string::npos){
        std::cout << "Tieu de khong duoc de trong!\n";
        return false;
    }

    if (input.find_first_of("/\\:*?\"<>|") != std::string::npos){
        std::cout << "Tieu de chua ky tu khong hop le!\n";
        return false;
    }

    if (input.back() == '.' || input.back() == ' '){
        std::cout << "Tieu de khong duoc ket thuc bang dau cham hoac dau cach!\n";
        return false;
    }
    if(input.front() == ' '){
        std::cout << "Tieu de khong duoc bat dau bang dau cach!\n";
        return false;
    }
    if(input.size() > 3 && equalsIgnoreCase(input.substr(input.size() - 3), ".md")){
        std::cout << "Tieu de khong can duoi .md!\n";
        return false;
    }
    return true;
}

bool NoteManager::isTitleExists(const std::string &title) const{
    return findIndex(title) != -1;
}



void NoteManager::loadNotesFromDisk(){
    fs::create_directories(NOTES_DIR);
    notesArray.clear();

    for (const auto &entry : fs::directory_iterator(NOTES_DIR)){
        if (!entry.is_regular_file() || entry.path().extension() != ".md") continue;

        std::string content;
        if(!readFileToString(entry.path(), content)) continue;
        

        notesArray.push_back(Note(entry.path().stem().string(), content, getFileTime(entry.path())));

    }
    linkIndexDirty = true;
    std::cout << "Da tai " << notesArray.size() << " ghi chu.\n";
}


bool NoteManager::deleteNote(const std::string &title){
    int i = findIndex(title);
    if (i == -1)
        return false;

    std::error_code ec;
    fs::remove(makePath(notesArray[i].getTitle()), ec);
    if (ec) {   // loi that su (bi khoa, khong co quyen...)
        std::cerr << "Khong the xoa file: " << ec.message() << std::endl;
        return false;
    }
    
    // Xoa khoi RAM
    notesArray.erase(notesArray.begin() + i);
    linkIndexDirty = true;
    return true;
}

int NoteManager::findIndex(const std::string& title) const{
    const std::string key = normalizeTitle(title);
    for(std::size_t i = 0; i < notesArray.size(); i++){
        if(equalsIgnoreCase(key, notesArray[i].getTitle())) return static_cast<int>(i);
    }
    return -1;
}

bool NoteManager::openNote(const std::string& title){
    int i = findIndex(title);
    if(i == -1) return false;

    if(!reloadNote(i)){
        std::cout << "(Khong doc lai duoc file, hien ban luu trong bo nho)\n";
    }
    notesArray[i].displayNote();
    return true;
}





// Cap nhat tren disk lan RAM
bool NoteManager::writeNoteContent(int i , const std::string& content){
    std::ofstream outFile(makePath(notesArray[i].getTitle()));
    outFile << content;
    outFile.close();
    if(outFile.fail()) return false;

    notesArray[i].setContent(content);
    notesArray[i].setLastModified(getCurrentTime());
    linkIndexDirty = true;
    return true;
}


bool NoteManager::editNote(const std::string& title, const std::string& newContent){
    int i = findIndex(title);
    if(i == -1) return false;
    return writeNoteContent(i, newContent);
}



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
    std::error_code ec;
    fs::rename(makePath(notesArray[i].getTitle()), makePath(newTitle), ec);
    if(ec){
        std::cerr << "Khong the doi ten file: " << ec.message() << std::endl;
        return false;
    }
    const std::string oldTitleStored = notesArray[i].getTitle();
    notesArray[i].setTitle(newTitle);
    notesArray[i].setLastModified(getCurrentTime());
    linkIndexDirty = true;

    int n = updateLinksAfterRename(oldTitleStored, newTitle);
    std::cout << "Da cap nhat link trong " << n << " ghi chu.\n";
    return true;
    //Đổi title trong RAM trước vòng lặp, để reloadNote của chính note vừa đổi tên đọc đúng path mới.
}


void NoteManager::debugDump() const{
    std::cout << "  [RAM] " << notesArray.size() << " ghi chu, linkIndexDirty="
              << (linkIndexDirty ? "true" : "false") << "\n";
    for(const Note& n : notesArray){
        std::cout << "    - \"" << n.getTitle() << "\" ("
                  << n.getContent().size() << " byte): "
                  << preview(n.getContent()) << "\n";
    }
    std::cout << std::flush;
}







