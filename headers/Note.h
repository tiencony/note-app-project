#ifndef NOTE_H
#define NOTE_H

#include <string>

class Note{
private:
    std::string title;
    std::string content;
    std::string lastModified;

public:
    Note();
    Note(const std::string& title, const std::string& content, const std::string& lastModified);

    const std::string &getTitle() const;
    void setTitle(const std::string &title);

    const std::string &getContent() const;
    void setContent(const std::string &content);

    const std::string &getLastModified() const;
    void setLastModified(const std::string &lastModified);

    void displayInfo() const;
    void displayNote() const;
};

#endif
