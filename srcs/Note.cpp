#include "Note.h"
#include <iostream>

Note::Note(){
    title = "Untitled";
    content = "";
    lastModified = "";
}

Note::Note(const std::string& title, const std::string& content, const std::string& lastModified){
    this->title = title;
    this->content = content;
    this->lastModified = lastModified;
}

const std::string &Note::getTitle() const{
    return title;
}

void Note::setTitle(const std::string &title){
    this->title = title;
}

const std::string &Note::getContent() const{
    return content;
}

void Note::setContent(const std::string &content){
    this->content = content;
}

const std::string &Note::getLastModified() const{
    return lastModified;
}


void Note::setLastModified(const std::string &lastModified){
    this->lastModified = lastModified;
}

