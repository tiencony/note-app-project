#include "../headers/NoteManager.h"
#include "../headers/Utils.h"

#include <fstream>
#include <filesystem>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <vector>

namespace fs = std::filesystem;
static const std::string NOTES_DIR = "vault/notes";

std::string NoteManager::makePath(const std::string &title) const{
    return NOTES_DIR + "/" + title + ".md";
}

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
    Note *temp = new Note[newCapacity];

    for (int i = 0; i < count; i++){
        temp[i] = notesArray[i];
    }

    delete[] notesArray;
    notesArray = temp;
    capacity = newCapacity;
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

    if (count >= capacity){
        resize();
    }

    notesArray[count] = Note(title, content, getCurrentTime());
    count++;
    return true;
}

void NoteManager::listNotes() const{
    if (count == 0) return;
    for (int i = 0; i < count; i++){
        notesArray[i].displayInfo();
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
    return true;
}

bool NoteManager::isTitleExists(const std::string &title) const{
    return findIndex(title) != -1;
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
    for (int j = i; j < count - 1; j++){
        notesArray[j] = notesArray[j + 1];
    }
    count--;
    return true;
}

int NoteManager::findIndex(const std::string &title) const{
    for (int i = 0; i < count; i++){
        if (equalsIgnoreCase(title, notesArray[i].getTitle()))
            return i;
    }
    return -1;
}

bool NoteManager::openNote(const std::string& title){
    int i = findIndex(title);
    if(i == -1) return false;

    if(!reloadNote(i)) return false;
    notesArray[i].displayNote();
    return true;
}



bool NoteManager::editNote(const std::string &title, const std::string &newContent){
    int i = findIndex(title);
    if (i == -1)
        return false;

    std::ofstream outFile(makePath(notesArray[i].getTitle()));
    if (!outFile){
        std::cerr << "Khong the mo file de ghi!\n";
        return false;
    }

    outFile << newContent;
    outFile.close();

    notesArray[i].setContent(newContent);
    notesArray[i].setLastModified(getCurrentTime());
    return true;
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
    notesArray[i].setTitle(newTitle);
    notesArray[i].setLastModified(getCurrentTime());
    return true;
}


std::vector<std::string> NoteManager::searchTitle(const std::string& keyword) const{
    std::vector<std::string> results;
    for(int i = 0; i < count; i++){
        if(containsIgnoreCase(notesArray[i].getTitle(), keyword)){
            results.push_back(notesArray[i].getTitle());
        }
    }
    return results;
}


std::vector<SearchResult> NoteManager::searchContent(const std::string& keyword) const{
    std::vector<SearchResult> results;
    if(keyword.empty()) return results;
    
    for(int i = 0; i < count; i++){
        const std::string& content = notesArray[i].getContent();
        std::size_t pos = findIgnoreCase(content, keyword);

        if(pos != std::string::npos){
            results.push_back({
                    notesArray[i].getTitle(),
                    makeSnippet(content, pos, keyword.size())
            });
        }
    }

    return results;
}


std::vector<std::string> NoteManager::listByTag(const std::string& tag) const{
    std::vector<std::string> results;
    std::string normalized_tag = normalizeTag(tag);
    
    for(int i = 0; i < count; i++){
        std::set<std::string> tags = extractTags(notesArray[i].getContent());
        if(tags.count(normalized_tag) > 0){
            results.push_back(notesArray[i].getTitle());

        }
    }
    return results;
}

std::map<std::string, int> NoteManager::tagCounts() const{
    std::map<std::string, int> counts;
    for(int i = 0; i < count; i++){
        std::set<std::string> tags = extractTags(notesArray[i].getContent());
        for(const std::string&tag : tags){
            counts[tag]++;
        }
    }
    return counts;
}




/* Chua xu ly vc doc tu RAM nen co the bi sai 
khi sua file bang Notepath (can sua lai sau)*/
std::vector<std::string> NoteManager::backlinks(const std::string& title) const{
    std::vector<std::string> results;
    for(int i = 0; i < count; i++){
        if(equalsIgnoreCase(notesArray[i].getTitle(), title)) continue;

        std::set<std::string> links = extractLinks(notesArray[i].getContent());
        for(const std::string& link : links){
            if(equalsIgnoreCase(link, title)){
                results.push_back(notesArray[i].getTitle());
                break;
            }
        }
    }
    return results;
}