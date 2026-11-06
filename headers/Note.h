#ifndef NOTE_H
#define NOTE_H


#include <string>  

class Note{
private:
    std::string title;
    std::string content;
    std::string createdTime;

public:
    Note();
    Note(std::string title, std::string content, std::string createTime);
    
    const std::string& getTitle() const;
    void setTitle(const std::string&  title);

    std::string getContent() const;
    void setContent(const std::string& content);

    std::string getCreatedTime() const;
    void setCreatedTime(const std::string& createdTime);

    void displayInfo() const;
    void displayNote() const;

};


#endif

