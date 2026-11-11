#include "NoteManager/NoteManager.h"
#include "Utils/Debug.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <system_error>
#include <vector>

namespace fs = std::filesystem;

static const std::string DIR    = "vault/notes";
static const std::string PREFIX = "zz_dbg_";
static bool gStep = false;

static void cleanup(){
    std::error_code ec;
    std::vector<fs::path> toDelete;
    for(const auto& e : fs::directory_iterator(DIR, ec)){
        if(e.path().filename().string().rfind(PREFIX, 0) == 0) toDelete.push_back(e.path());
    }
    for(const auto& p : toDelete) fs::remove(p, ec);
}

// In RAM va dia canh nhau, tuy chon dung lai cho Enter
static void snapshot(const std::string& label, const NoteManager& m){
    std::cout << "\n=== " << label << " ===\n";
    m.debugDump();
    dumpDisk(DIR, PREFIX);
    if(gStep){
        std::cout << "(nhan Enter de di tiep) " << std::flush;
        std::string s;
        std::getline(std::cin, s);
    }
}



int main(){
    NoteManager m;
    m.createNote("zz_dbg_A", "hoc con tro");
    m.createNote("zz_dbg_B", "ke hoach tuan");
    snapshot("Truoc khi doi ten", m);

    m.renameNote("zz_dbg_B", "zz_dbg_B2");
    snapshot("Sau khi doi ten B -> B2", m);
    return 0;
}