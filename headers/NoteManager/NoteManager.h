#ifndef NOTE_MANAGER_H
#define NOTE_MANAGER_H

#include "Note.h"
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

struct SearchResult {
    std::string title;
    std::string snippet;
};

struct BrokenLink{
    std::string from;     // note chua link
    std::string target;   // ten ma link tro toi (khong ton tai)
};

class NoteManager{
private:
    std::vector<Note> notesArray;

    std::string makePath(const std::string &title) const;
    bool isTitleExists(const std::string &title) const;
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
    int updateLinksAfterRename(const std::string& oldTitle, const std::string& newTitle);
public:
    NoteManager() = default;


    bool createNote(const std::string &title = "Untitled", const std::string &content = "No content");
    void listNotes() const;
    bool checkTitle(const std::string &input) const;


    void loadNotesFromDisk();

    bool deleteNote(const std::string &title);
    bool openNote(const std::string &title);

    
    bool editNote(const std::string &title, const std::string &newContent);
    bool renameNote(const std::string& oldTitle, const std::string& newTitle);


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