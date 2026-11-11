#include "NoteManager/NoteManager.h"
#include "Utils/Parser.h"        // extractLinks, 
#include "Utils/StringUtils.h"   // normalizeTitle
#include <iostream>


void NoteManager::rebuildLinkIndex() const{
    backlinksByTitle.clear();
    linksBySource.clear();
    titleByKey.clear();

    // Register all note titles first, so broken-link checks can resolve them.
    for (const Note& note : notesArray) {
        titleByKey[normalizeTitle(note.getTitle())] = note.getTitle();
    }

    for (const Note& note : notesArray) {
        const std::string sourceKey = normalizeTitle(note.getTitle());

        for (const std::string& link : extractLinks(note.getContent())) {
            const std::string targetKey = normalizeTitle(link);
            if (targetKey.empty()) continue;

            linksBySource[sourceKey].insert(link);
            backlinksByTitle[targetKey].insert(sourceKey);
        }
    }
}


// [RAM] Tra hash, nhung cache co the cu neu file bi sua ngoai chuong trinh.
std::vector<std::string> NoteManager::backlinks(const std::string& title) const{
    ensureLinkIndex();   
    std::vector<std::string> results;
    const std::string targetKey = normalizeTitle(title);

    auto it = backlinksByTitle.find(targetKey);
    if(it == backlinksByTitle.end()) return results;

    for(const std::string& sourceKey : it->second){
        if(sourceKey == targetKey) continue;
        auto titleIt = titleByKey.find(sourceKey);
        if(titleIt != titleByKey.end()){
            results.push_back(titleIt->second);
        }
    }
    return results;
}


std::vector<BrokenLink> NoteManager::brokenLinks() const{
    ensureLinkIndex();
    std::vector<BrokenLink> results;

    for(const Note& note : notesArray){
        auto it = linksBySource.find(normalizeTitle(note.getTitle()));
        if(it == linksBySource.end()) continue;

        std::set<std::string> reported;   // cac dich (theo khoa) da bao trong note nay
        for(const std::string& link : it->second){
            const std::string key = normalizeTitle(link);

            if(titleByKey.count(key) > 0) continue;        // dich ton tai -> khong hong
            if(!reported.insert(key).second) continue;     // da bao dich nay roi -> bo qua

            results.push_back({note.getTitle(), link});
        }
    }
    return results;
}

void NoteManager::ensureLinkIndex() const{
    if(linkIndexDirty){
        rebuildLinkIndex();
        linkIndexDirty = false;
    }
}


int NoteManager::updateLinksAfterRename(const std::string& oldTitle, const std::string& newTitle){
    if(normalizeTitle(oldTitle) == normalizeTitle(newTitle)) return 0;
    int updated = 0;
    for(std::size_t i = 0; i < notesArray.size(); i++){
        const int idx = static_cast<int>(i);
        if(!reloadNote(idx)) continue; // Ko doc dc thi bo qua, ko ghi de


    
        const std::string replaced = replaceLink(notesArray[i].getContent(), oldTitle, newTitle);
        if(replaced == notesArray[i].getContent()) continue;
        
        if(writeNoteContent(idx, replaced)) updated++;
        else std::cerr << "Khong cap nhap duoc link trong: " << notesArray[i].getTitle() << std::endl;

    }
    return updated;
} 