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
    for(std::size_t i = 0; i < notesArray.size(); i++){
        if(equalsIgnoreCase(title, notesArray[i].getTitle())) return static_cast<int>(i);
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
    linkIndexDirty = true;
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
    linkIndexDirty = true;
    notesArray[i].setLastModified(getCurrentTime());
    return true;
}


std::vector<std::string> NoteManager::searchTitle(const std::string& keyword) const{
    std::vector<std::string> results;
    if(keyword.empty()) return results;
    for(const Note& note : notesArray){
        if(containsIgnoreCase(note.getTitle(), keyword)){
            results.push_back(note.getTitle());
        }
    }
    return results;
}


std::vector<SearchResult> NoteManager::searchContent(const std::string& keyword) const{
    std::vector<SearchResult> results;
    if(keyword.empty()) return results;
    
    for(const Note& note : notesArray){
        const std::string& content = note.getContent();
        std::size_t pos = findIgnoreCase(content, keyword);

        if(pos != std::string::npos){
            results.push_back({
                    note.getTitle(),
                    makeSnippet(content, pos, keyword.size())
            });
        }
    }

    return results;
}


std::vector<std::string> NoteManager::listByTag(const std::string& tag) const{
    std::vector<std::string> results;
    std::string normalized_tag = normalizeTag(tag);
    
    for(const Note& note : notesArray){
        std::set<std::string> tags = extractTags(note.getContent());
        if(tags.count(normalized_tag) > 0){
            results.push_back(note.getTitle());

        }
    }
    return results;
}

std::map<std::string, int> NoteManager::tagCounts() const{
    std::map<std::string, int> counts;
    for(const Note& note : notesArray){
        std::set<std::string> tags = extractTags(note.getContent());
        for(const std::string&tag : tags){
            counts[tag]++;
        }
    }
    return counts;
}




void NoteManager::rebuildLinkIndex() const{
    backlinksByTitle.clear();
    linksBySource.clear();
    titleByKey.clear();

    // Register all note titles first, so broken-link checks can resolve them.
    for (const Note& note : notesArray) {
        titleByKey[normalizeTitle(note.getTitle())] = note.getTitle();
    }

    for (const Note& note : notesArray) {
        const std::string sourceKey = normalizeTitle(note.getTitle());

        for (const std::string& link : extractLinks(note.getContent())) {
            const std::string targetKey = normalizeTitle(link);
            if (targetKey.empty()) continue;

            linksBySource[sourceKey].insert(link);
            backlinksByTitle[targetKey].insert(sourceKey);
        }
    }
}


// [RAM] Tra hash, nhung cache co the cu neu file bi sua ngoai chuong trinh.
std::vector<std::string> NoteManager::backlinks(const std::string& title) const{
    ensureLinkIndex();   
    std::vector<std::string> results;
    const std::string targetKey = normalizeTitle(title);

    auto it = backlinksByTitle.find(targetKey);
    if(it == backlinksByTitle.end()) return results;

    for(const std::string& sourceKey : it->second){
        if(sourceKey == targetKey) continue;
        auto titleIt = titleByKey.find(sourceKey);
        if(titleIt != titleByKey.end()){
            results.push_back(titleIt->second);
        }
    }
    return results;
}


std::vector<BrokenLink> NoteManager::brokenLinks() const{
    ensureLinkIndex();
    std::vector<BrokenLink> results;
    for(const Note& note : notesArray){          
        auto it = linksBySource.find(normalizeTitle(note.getTitle()));
        if(it == linksBySource.end()) continue;

        for(const std::string& link : it->second){
            if(titleByKey.find(normalizeTitle(link)) == titleByKey.end()){
                results.push_back({note.getTitle(), link});
            }
        }
    }
    return results;                             
}


void NoteManager::ensureLinkIndex() const{
    if(linkIndexDirty){
        rebuildLinkIndex();
        linkIndexDirty = false;
    }
}