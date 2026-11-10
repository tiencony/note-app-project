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

int main(int argc, char* argv[]){
    gStep = (argc > 1 && std::string(argv[1]) == "--step");
    cleanup();

    NoteManager m;
    m.createNote("zz_dbg_A", "hoc con tro");
    m.createNote("zz_dbg_B", "ke hoach tuan");
    m.createNote("zz_dbg_C", "xem [[zz_dbg_A]]");
    snapshot("1. Sau khi tao A, B, C", m);

    std::cout << "\n>>> renameNote(B -> B2)\n";
    bool ok = m.renameNote("zz_dbg_B", "zz_dbg_B2");
    std::cout << "ket qua: " << (ok ? "true" : "false") << "\n";
    snapshot("2. Sau khi doi ten B -> B2", m);

    // Mo phong ban sua file bang Notepad: RAM chua biet
    {
        std::ofstream out(DIR + "/zz_dbg_C.md");
        out << "xem [[zz_dbg_A]] (them bang Notepad)";
    }
    snapshot("3. Sau khi sua C ben ngoai (RAM con ban cu, dia la ban moi)", m);

    std::cout << "\n>>> renameNote(A -> A2)\n";
    ok = m.renameNote("zz_dbg_A", "zz_dbg_A2");
    std::cout << "ket qua: " << (ok ? "true" : "false") << "\n";
    snapshot("4. Sau khi doi ten A -> A2", m);

    auto broken = m.brokenLinks();
    std::cout << "\nLink hong: " << broken.size() << "\n";
    for(const auto& b : broken) std::cout << "  " << b.from << " -> [[" << b.target << "]]\n";

    cleanup();
    return 0;
}