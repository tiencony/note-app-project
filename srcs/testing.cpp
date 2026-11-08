#include "../headers/NoteManager.h"
#include "../headers/Utils.h"
#include "../headers/Note.h"

#include <fstream>
#include <filesystem>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <set>
#include <map>

namespace fs = std::filesystem;
using Tags = std::set<std::string>;



/* Chua xu ly vc doc tu RAM nen co the bi sai 
khi sua file bang Notepath (can sua lai sau)*/
std::vector<std::string> NoteManager::backlinks(const std::string& title) const{
    std::vector<std::string> results;
    for(int i = 0; i < count; i++){
        if(equalsIgnoreCase(notesArray[i].getTitle(), title)) continue;

        std::set<std::string> links = extractLinks(notesArray[i].getContent());
        for(const std::string& link : links){
            if(equalsIgnoreCase(link, title)){
                results.push_back(notesArray[i].getTitle());
                break;
            }
        }
    }
    return results;
}