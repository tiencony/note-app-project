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
#include <utility>

namespace fs = std::filesystem;

NoteManager::NoteManager(std::filesystem::path rootDir) : root(std::move(rootDir)) {}


std::string NoteManager::makePath(const std::string& title) const{
    return (root / (title + ".md")).string();
}



bool NoteManager::reloadNote(int i){
    std::string path = makePath(notesArray[i].getTitle());
    std::string content;
    if(!readFileToString(path, content)) return false; 

    if(notesArray[i].getContent() != content) linkIndexDirty = true;
    notesArray[i].setContent(content);
    notesArray[i].setLastModified(getFileTime(path));
    return true;

}



NoteError NoteManager::validateNewTitle(const std::string& title) const{
    if(!checkTitle(title)) return NoteError::InvalidTitle;
    if(isTitleExists(title)) return NoteError::AlreadyExists;
    std::error_code ec;
    if(fs::exists(makePath(title), ec)) return NoteError::AlreadyExists;
    return NoteError::Ok;
}



NoteError NoteManager::createNote(const std::string &title, const std::string &content){
    NoteError v = validateNewTitle(title);
    if(v != NoteError::Ok) return v;



    std::error_code ec;
    fs::create_directories(root, ec);
    if(ec) return NoteError::WriteFailed;

    const std::string filePath = makePath(title);    
    std::ofstream outFile(filePath);
    if(!outFile) return NoteError::WriteFailed;

    outFile << content;
    outFile.close();
    if(outFile.fail()){                                                  // loi luc ghi (dia day...)
        fs::remove(filePath, ec);                                        // don file do dang
        return NoteError::WriteFailed;
    }


    notesArray.push_back(Note(title, content, getCurrentTime()));
    linkIndexDirty = true;
    return NoteError::Ok;

}

std::vector<NoteInfo> NoteManager::listNotes() const{
    std::vector<NoteInfo> result;
    result.reserve(notesArray.size());//Bt trc kich co nen dung reserve cho nhanh
    for(const Note& note : notesArray){
        result.push_back({note.getTitle(), note.getLastModified()});

    }
    return result;
}


bool NoteManager::checkTitle(const std::string &input) const{
    if (input.empty() || input.find_first_not_of(' ') == std::string::npos) return false;
    if (input.find_first_of("/\\:*?\"<>|") != std::string::npos) return false;
    if (input.back() == '.' || input.back() == ' ') return false;
    if(input.front() == ' ') return false;
    if(input.size() > 3 && equalsIgnoreCase(input.substr(input.size() - 3), ".md")) return false;
    return true;
}

bool NoteManager::isTitleExists(const std::string &title) const{
    return findIndex(title) != -1;
}



LoadReport NoteManager::loadNotesFromDisk(){
    LoadReport report;
    std::error_code ec;
    fs::create_directories(root, ec);
    if(ec){
        report.ok = false;
        return report;
    }
    std::vector<Note> fresh;
    for(fs::directory_iterator it(root, ec), end; !ec && it != end; it.increment(ec)){
        std::error_code ec2;
        if(!it->is_regular_file(ec2) || it->path().extension() != ".md") continue;

        std::string content;
        if(!readFileToString(it->path(), content)){
            report.skipped.push_back(it->path().filename().string());
            continue;
        }
        fresh.push_back(Note(it->path().stem().string(), content, getFileTime(it->path())));
    }
    if(ec){
        report.ok = false; // loi giua chung: RAM van nguyen
        return report;
    }
    notesArray = std::move(fresh);
    linkIndexDirty = true;
    report.loaded = static_cast<int>(notesArray.size());
    return report;

}


NoteError NoteManager::deleteNote(const std::string &title){
    int i = findIndex(title);
    if (i == -1) return NoteError::NotFound;

    std::error_code ec;
    fs::remove(makePath(notesArray[i].getTitle()), ec);
    if (ec) return NoteError::DeleteFailed;
    
    // Xoa khoi RAM
    notesArray.erase(notesArray.begin() + i);
    linkIndexDirty = true;
    return NoteError::Ok;
}

int NoteManager::findIndex(const std::string& title) const{
    const std::string key = normalizeTitle(title);
    for(std::size_t i = 0; i < notesArray.size(); i++){
        if(equalsIgnoreCase(key, notesArray[i].getTitle())) return static_cast<int>(i);
    }
    return -1;
}


// Thu doc lai tu DISK de lam moi cache RAM. 
// Neu file mat/loi -> stale = true, van tra ban RAM cu (ko xoa khoi RAM)
OpenResult NoteManager::openNote(const std::string& title){
    OpenResult result;
    int i = findIndex(title);
    if(i == -1){
        result.error = NoteError::NotFound;
        return result;
    }
    result.stale = !reloadNote(i); // stale = true neu ko doc duoc tu dia
    result.note = notesArray[i];   // Luon tra ban sao an toan
    return result;
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


NoteError NoteManager::editNote(const std::string& title, const std::string& newContent){
    int i = findIndex(title);
    if(i == -1) return NoteError::NotFound;
    fs::path targetPath = makePath(notesArray[i].getTitle());
    fs::path tempPath = targetPath;
    tempPath += ".tmp";
    std::error_code ec;

    {
        std::ofstream out(tempPath);
        if(!out.is_open()){
            return NoteError::WriteFailed;
        }
        out << newContent;
    }
    fs::rename(tempPath, targetPath, ec);
    if(ec){
        fs::remove(tempPath, ec); // Nếu đổi tên lỗi thì dọn file tạm
        return NoteError::WriteFailed;
    }
    return writeNoteContent(i, newContent) ? NoteError::Ok : NoteError::WriteFailed;
}



NoteError NoteManager::renameNote(const std::string& oldTitle, const std::string& newTitle){
    int i = findIndex(oldTitle);
    if(i == -1) return NoteError::NotFound;
    if(!checkTitle(newTitle)) return NoteError::InvalidTitle;

    const std::string oldStored = notesArray[i].getTitle();
    std::error_code ec;

    // Cho phep doi chu cai hoa/thuong voi chinh no
    int j = findIndex(newTitle);
    if(j != -1 && j != i) return NoteError::AlreadyExists;

    if(j == -1 && fs::exists(makePath(newTitle), ec)) return NoteError::AlreadyExists;

    fs::rename(makePath(oldStored), makePath(newTitle), ec);
    if(ec) return NoteError::WriteFailed;


    notesArray[i].setTitle(newTitle);
    notesArray[i].setLastModified(getCurrentTime());
    linkIndexDirty = true;

    updateLinksAfterRename(oldStored, newTitle);   
    return NoteError::Ok;
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







