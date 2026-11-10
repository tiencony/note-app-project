#include "NoteManager/NoteManager.h"
#include "Utils/Parser.h"        // extractTags, normalizeTag



std::vector<std::string> NoteManager::listByTag(const std::string& tag) const{
    std::vector<std::string> results;
    std::string normalized_tag = normalizeTag(tag);
    
    for(const Note& note : notesArray){
        std::set<std::string> tags = extractTags(note.getContent());
        if(tags.count(normalized_tag) > 0){
            results.push_back(note.getTitle());

        }
    }
    return results;
}

std::map<std::string, int> NoteManager::tagCounts() const{
    std::map<std::string, int> counts;
    for(const Note& note : notesArray){
        std::set<std::string> tags = extractTags(note.getContent());
        for(const std::string&tag : tags){
            counts[tag]++;
        }
    }
    return counts;
}