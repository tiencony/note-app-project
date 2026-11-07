#ifndef NOTE_MANAGER_H
#define NOTE_MANAGER_H

#include "Note.h"
#include <string>
#include <vector>

struct SearchResult {
    std::string title;
    std::string snippet;
};



class NoteManager{
private:
    Note *notesArray;
    int capacity;
    int count;

    void resize();
    std::string makePath(const std::string &title) const;
    bool isTitleExists(const std::string &title) const;
    int findIndex(const std::string &title) const;

public:
    NoteManager(int initialCapacity = 10);
    ~NoteManager();

    bool createNote(const std::string &title = "Untitled", const std::string &content = "No content");
    void listNotes() const;
    bool checkTitle(const std::string &input) const;
    NoteManager(const NoteManager &) = delete;
    NoteManager &operator=(const NoteManager &) = delete;

    void loadNotesFromDisk();

    bool deleteNote(const std::string &title);
    bool openNote(const std::string &title) const;

    
    bool editNote(const std::string &title, const std::string &newContent);
    bool renameNote(const std::string& oldTitle, const std::string& newTitle);

    std::vector<SearchResult> searchContent(const std::string& keyword) const;


};

#endif