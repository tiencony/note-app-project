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

std::map<std::string, int> NoteManager::tagCounts() const{
    std::map<std::string, int> counts;
    for(int i = 0; i < count; i++){
        std::set<std::string> tags = extractTags(notesArray[i].getContent());
        for(const std::string&tag : tags){
            counts[tag]++;
        }
    }
    return counts;
}







int main(){
    for(const auto& t : extractTags("hoc #CPP va #ooP1rw, #CPP\n# Tieu de\nabc#def")){
        std::cout << normalizeTag(t) << std::endl;
    }
}