#include "../headers/NoteManager.h"
#include "../headers/Utils.h"
#include "../headers/Note.h"

#include <fstream>
#include <filesystem>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <vector>

namespace fs = std::filesystem;

std::vector<SearchResult> NoteManager::searchContent(const std::string& keyword) const{
    std::vector<SearchResult> results;
    if(keyword.empty()) return results;
    
    for(int i = 0; i < count; i++){
        const std::string& content = notesArray[i].getContent();
        std::size_t pos = findIgnoreCase(content, keyword);

        if(pos != std::string::npos){
            results.push_back({
                    notesArray[i].getTitle(),
                    makeSnippet(content, pos, keyword.size())
            });
        }
    }

    return results;
}

