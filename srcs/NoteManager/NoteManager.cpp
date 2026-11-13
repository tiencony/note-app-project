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
    std::sort(result.begin(), result.end(), [](const NoteInfo& a, const NoteInfo& b){
        return normalizeTitle(a.title) < normalizeTitle(b.title);
    });
    return result;
}


bool NoteManager::checkTitle(const std::string &input) const{
    if (input.empty() || input.find_first_not_of(' ') == std::string::npos) return false;
    if (input.find_first_of("/\\:*?\"<>|#^[]") != std::string::npos) return false;
    if (input.back() == '.' || input.back() == ' ') return false;
    if(input.front() == ' ') return false;
    if(input.size() > 3 && equalsIgnoreCase(input.substr(input.size() - 3), ".md")) return false;
    return true;
}

bool NoteManager::isTitleExists(const std::string &title) const{
    return findIndex(title) != -1;
}


NoteError NoteManager::deleteNote(const std::string &title){
    int i = findIndex(title);
    if (i == -1) return NoteError::NotFound;

    // Xoa tren disk
    std::error_code ec;
    fs::remove(makePath(notesArray[i].getTitle()), ec);
    if (ec) return NoteError::DeleteFailed;
    
    // Xoa khoi RAM
    notesArray.erase(notesArray.begin() + i);
    linkIndexDirty = true;
    return NoteError::Ok;
}


// Tim o tren RAM
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
    if(!writeFileAtomic(makePath(notesArray[i].getTitle()), content)) return false;
    
    notesArray[i].setContent(content);
    notesArray[i].setLastModified(getCurrentTime());
    linkIndexDirty = true;
    return true;
}


NoteError NoteManager::editNote(const std::string& title, const std::string& newContent){
    int i = findIndex(title);
    if(i == -1) return NoteError::NotFound;
    return writeNoteContent(i, newContent) ? NoteError::Ok : NoteError::WriteFailed;
}




RenameResult NoteManager::renameNote(const std::string& oldTitle, const std::string& newTitle){
    RenameResult result;
    auto fail = [&result](NoteError e){result.error = e; return result;};



    int i = findIndex(oldTitle);
    if(i == -1) return fail(NoteError::NotFound);
    if(!checkTitle(newTitle)) return fail(NoteError::InvalidTitle);

    const std::string oldStored = notesArray[i].getTitle();
    std::error_code ec;

    // Cho phep doi chu cai hoa/thuong voi chinh no
    int j = findIndex(newTitle);
    if(j != -1 && j != i) return fail(NoteError::AlreadyExists);

    if(j == -1 && fs::exists(makePath(newTitle), ec)) return fail(NoteError::AlreadyExists);

    fs::rename(makePath(oldStored), makePath(newTitle), ec);
    if(ec) return fail(NoteError::WriteFailed);


    notesArray[i].setTitle(newTitle);
    notesArray[i].setLastModified(getFileTime(makePath(newTitle)));
    linkIndexDirty = true;

    updateLinksAfterRename(oldStored, newTitle, result);   
    return result;
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




LoadReport NoteManager::loadNotesFromDisk(){
    LoadReport report;
    
    std::vector<Note> fresh;
    if(!scanDisk(fresh, report.skipped)){
        report.ok = false;
        return report;
    }

    notesArray = std::move(fresh);
    linkIndexDirty = true;
    report.loaded = static_cast<int>(notesArray.size());
    return report;
}



bool NoteManager::scanDisk(std::vector<Note>& out, std::vector<std::string>& skipped){
    std::error_code ec;
    fs::create_directories(root, ec);
    if(ec) return false;

    for(fs::directory_iterator it(root, ec), end; !ec && it != end; it.increment(ec)){
        std::error_code ec2;
        if(!it->is_regular_file(ec2) || it->path().extension() != ".md") continue;

        const std::string stem = it->path().stem().string();
        std::string content;
        if(!checkTitle(stem) || !readFileToString(it->path(), content)){
            skipped.push_back(it->path().filename().string());
            continue;
        }
        out.push_back(Note(stem, content, getFileTime(it->path())));
    }
    return !ec; //Neu khong loi j thi tra ve true
}


SyncReport NoteManager::refreshFromDisk(){
    SyncReport rep;
    std::vector<Note> fresh;
    if(!scanDisk(fresh, rep.skipped)){
        rep.ok = false;
        return rep;
    }
    
    // Duyet RAM
    std::unordered_map<std::string, const Note*> oldByKey;
    oldByKey.reserve(notesArray.size());
    for(const Note& note : notesArray){
        oldByKey[normalizeTitle(note.getTitle())] = &note;
    }

    // Duyet o dia
    std::set<std::string> seen;
    for(const Note& freshNote : fresh){
        std::string key = normalizeTitle(freshNote.getTitle());
        seen.insert(key);
        
        auto it = oldByKey.find(key);
        if(it == oldByKey.end()){
            rep.added.push_back(freshNote.getTitle());
        }else if(it->second->getContent() != freshNote.getContent()){
            rep.changed.push_back(freshNote.getTitle());
        }

    }
    for(const Note& note : notesArray){
        if(seen.count(normalizeTitle(note.getTitle())) == 0){
            rep.removed.push_back(note.getTitle());
        }
    }
    if(rep.hasChanges()){
        notesArray = std::move(fresh);
        linkIndexDirty = true;
    }
    return rep;

}