#ifndef NOTE_MANAGER_H
#define NOTE_MANAGER_H

#include "Note.h"
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>
#include <filesystem>

struct SearchResult {
    std::string title;
    std::string snippet;
};

struct BrokenLink{
    std::string from;     // note chua link
    std::string target;   // ten ma link tro toi (khong ton tai)
};

struct LoadReport {
    bool ok = true;        // false: khong duyet duoc thu muc -> RAM giu nguyen
    int loaded = 0;                    
    std::vector<std::string> skipped;   // ten file khong doc duoc
};

enum class NoteError {
    Ok,
    NotFound,
    InvalidTitle,
    AlreadyExists,
    ReadFailed,
    WriteFailed,
    DeleteFailed 
};

struct OpenResult{
    NoteError error = NoteError::Ok;
    Note note;
    bool stale = false;   //true: ko doc lai duoc tu dia -> doc lai trg RAM
};

struct NoteInfo {
    std::string title;
    std::string lastModified;
};

struct RenameResult {
    NoteError error = NoteError::Ok;
    int updatedNotes = 0;                    // so note da duoc cap nhat link
    std::vector<std::string> failedNotes;    // note ghi link that bai (file cu van giu nguyen)
};

class NoteManager{
private:
    std::vector<Note> notesArray;
    std::filesystem::path root; 

    std::string makePath(const std::string &title) const;
    int findIndex(const std::string &title) const;
    bool reloadNote(int i);
    
    //Phan link
    mutable bool linkIndexDirty = true;
    mutable std::unordered_map<std::string, std::string> titleByKey; /*(Key): tieu de da chuan hoa, (Val): Tieu de goc*/
    mutable std::unordered_map<std::string, std::set<std::string>> backlinksByTitle;
    mutable std::unordered_map<std::string, std::set<std::string>> linksBySource; // Outgoing links
    
    void rebuildLinkIndex() const;
    void ensureLinkIndex() const;

    bool writeNoteContent(int i , const std::string& content);
    void updateLinksAfterRename(const std::string& oldTitle, const std::string& newTitle, RenameResult& result);
public:
    explicit NoteManager(std::filesystem::path rootDir = "vault/notes");

    NoteError createNote(const std::string &title = "Untitled",
                     const std::string &content = "No content");

    NoteError validateNewTitle(const std::string& title) const;
    bool isTitleExists(const std::string& title) const;
    std::vector<NoteInfo> listNotes() const; 
    bool checkTitle(const std::string &input) const;


    LoadReport loadNotesFromDisk();    

    OpenResult openNote(const std::string &title);

    NoteError editNote(const std::string &title, const std::string &newContent);
    NoteError deleteNote(const std::string &title);
    RenameResult renameNote(const std::string& oldTitle, const std::string& newTitle);

    //search
    std::vector<SearchResult> searchContent(const std::string& keyword) const;
    std::vector<std::string> searchTitle(const std::string& keyword) const;


    //tag
    std::vector<std::string> listByTag(const std::string& tag) const;
    std::map<std::string, int> tagCounts() const;

    //backlinks
    std::vector<std::string> backlinks(const std::string& title) const;
    std::vector<BrokenLink> brokenLinks() const;

    void debugDump() const;   // in trang thai trong RAM (de debug)
};

#endif