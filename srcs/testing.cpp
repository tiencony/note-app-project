#include "../headers/NoteManager.h"
#include "../headers/Utils.h"

#include <fstream>
#include <filesystem>
#include <iostream>
#include <algorithm>
#include <cctype>

void NoteManager::searchTitle(const std::string& keyword) const{
    bool found = false;
    for(int i = 0; i < count; i++){
        if(containsIgnoreCase(notesArray[i].getTitle(), keyword)){
            notesArray[i].displayInfo();
            found = true;
        }
    }
    if(!found){
        std::cout << "Khong tim duoc file chua tu khoa: " << keyword << std::endl;
    }
}
