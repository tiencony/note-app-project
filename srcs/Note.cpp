#include "../headers/Note.h"
#include <iostream>

Note::Note(){
    title = "Untitled";
    content = "";
    createdTime = "";
}   

Note::Note(std::string title, std::string content, std::string createdTime){
    this->title = title;
    this->content = content;
    this->createdTime = createdTime;
}

const std::string& Note::getTitle() const{
    return title;
}
void Note::setTitle(const std::string& title){
    this->title = title;
}

std::string Note::getContent() const{
    return content;
}
void Note::setContent(const std::string& content){
    this->content = content;
}

std::string Note::getCreatedTime() const{
    return createdTime;
}
void Note::setCreatedTime(const std::string& createdTime){
    this->createdTime = createdTime;
}



void Note::displayInfo() const{
    std::cout << "-------------------------------------------\n";
    std::cout << "Title: " << title << " (" << createdTime << ")\n";
}

void Note::displayNote() const{
    std::cout << "File content:\n" << content << std::endl;
}
