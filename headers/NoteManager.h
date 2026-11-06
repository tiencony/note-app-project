#ifndef NOTE_MANAGER_H
#define NOTE_MANAGER_H

#include "Note.h"
#include <string>

class NoteManager{
private:
    Note* notesArray;
    int capacity;
    int count;

    void resize();

public:
    NoteManager(int initialCapacity = 10);
    ~NoteManager();
    bool createNote(const std::string& title = "Untitled", const std::string& content = "No content");
    void listNotes() const;
    void searchTitle(const std::string& keyword) const;
    bool isTitleExists(const std::string& title) const;
    bool checkTitle(const std::string& input) const;
    NoteManager(const NoteManager&) = delete;
    NoteManager& operator=(const NoteManager&) = delete;

};


#endif