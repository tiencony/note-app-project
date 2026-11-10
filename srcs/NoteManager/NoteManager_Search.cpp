#include "NoteManager/NoteManager.h"
#include "Utils/StringUtils.h"   // containsIgnoreCase, findIgnoreCase, makeSnippet



std::vector<std::string> NoteManager::searchTitle(const std::string& keyword) const{
    std::vector<std::string> results;
    const std::string key = trimSpaces(keyword);
    if(key.empty()) return results;
    for(const Note& note : notesArray){
        if(containsIgnoreCase(note.getTitle(), key)){
            results.push_back(note.getTitle());
        }
    }
    return results;
}


std::vector<SearchResult> NoteManager::searchContent(const std::string& keyword) const{
    std::vector<SearchResult> results;
    const std::string key = trimSpaces(keyword);
    if(key.empty()) return results;
    
    for(const Note& note : notesArray){
        const std::string& content = note.getContent();
        std::size_t pos = findIgnoreCase(content, key);

        if(pos != std::string::npos){
            results.push_back({
                    note.getTitle(),
                    makeSnippet(content, pos, key.size())
            });
        }
    }

    return results;
}