// ============================================================================
// NoteManager.h - Quan ly tap ghi chu: dia la NGUON SU THAT, RAM la cache.
//
// MO HINH   : notesArray (RAM) phan anh thu muc `root`. Sua ngoai chuong trinh
//             thi RAM cu cho den khi goi openNote / loadNotesFromDisk /
//             refreshFromDisk.
// BAT BIEN  : - Moi title trong notesArray hop le (checkTitle) va duy nhat
//               theo normalizeTitle.
//             - Moi thay doi RAM lam thay doi noi dung/title phai dat
//               linkIndexDirty = true.
// BO CUC FILE (cung mot class):
//   NoteManager.cpp        CRUD, load/refresh, rename
//   NoteManager_Links.cpp  index link: backlinks, brokenLinks, cap nhat link khi rename
//   NoteManager_Search.cpp searchTitle / searchContent
//   NoteManager_Tags.cpp   listByTag / tagCounts
// BAO LOI   : NoteError / struct ket qua. KHONG nem exception. Cac ham tra ket
//             qua can duoc kiem tra (failedNotes, stale, skipped...).
// LIFETIME  : Khong co ham nao tra tham chieu/con tro vao notesArray; openNote
//             tra BAN SAO. Giu nguyen quy uoc nay.
// THREAD    : KHONG thread-safe. Cac ham const van sua duoc index (mutable).
// ============================================================================

// -------------------- PHAN LINK ------------------------
// Index link (lazy). Ba map mutable duoc dung lai khi linkIndexDirty == true.
//   titleByKey        : key chuan hoa -> title goc
//   linksBySource     : key note nguon -> link di ra (ten goc trong [[...]])
//   backlinksByTitle  : key dich -> tap key note nguon
// Quy uoc: moi key o day deu di qua normalizeTitle().
// Hien rebuild TOAN BO moi lan dirty (O(tong dung luong)); can tang dan neu vault lon.




#ifndef NOTE_MANAGER_H
#define NOTE_MANAGER_H

#include "Note.h"
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>
#include <filesystem>

struct SearchResult {
    std::string title;
    std::string snippet;
};

struct BrokenLink{
    std::string from;     // note chua link
    std::string target;   // ten ma link tro toi (khong ton tai)
};

struct LoadReport {
    bool ok = true;        // false: khong duyet duoc thu muc -> RAM giu nguyen
    int loaded = 0;                    
    std::vector<std::string> skipped;   // ten file khong doc duoc
};

enum class NoteError {
    Ok,
    NotFound,
    InvalidTitle,
    AlreadyExists,
    ReadFailed,
    WriteFailed,
    DeleteFailed 
};

struct OpenResult{
    NoteError error = NoteError::Ok;
    Note note;
    bool stale = false;   //true: ko doc lai duoc tu dia -> doc lai trg RAM
};

struct NoteInfo {
    std::string title;
    std::string lastModified;
};

struct RenameResult {
    NoteError error = NoteError::Ok;
    int updatedNotes = 0;                    // so note da duoc cap nhat link
    std::vector<std::string> failedNotes;    // note ghi link that bai (file cu van giu nguyen)
};



struct SyncReport {
    bool ok = true;
    std::vector<std::string> added;   // Các ghi chú mới xuất hiện trên đĩa
    std::vector<std::string> removed; // Các ghi chú đã bị xóa ngoài đĩa
    std::vector<std::string> changed; // Các ghi chú có nội dung thay đổi so với RAM
    std::vector<std::string> skipped; // Các file rác hoặc tên không hợp lệ bị bỏ qua

    bool hasChanges() const {
        return !added.empty() || !removed.empty() || !changed.empty();
    }
};

class NoteManager{
private:
    std::vector<Note> notesArray;
    std::filesystem::path root; 

    std::string makePath(const std::string &title) const;
    int findIndex(const std::string &title) const;
    bool reloadNote(int i);
    
    //Phan link
    mutable bool linkIndexDirty = true;
    mutable std::unordered_map<std::string, std::string> titleByKey; /*(Key): tieu de da chuan hoa, (Val): Tieu de goc*/
    mutable std::unordered_map<std::string, std::set<std::string>> backlinksByTitle;
    mutable std::unordered_map<std::string, std::set<std::string>> linksBySource; // Outgoing links
    
    void rebuildLinkIndex() const;
    void ensureLinkIndex() const;

    bool writeNoteContent(int i , const std::string& content);
    void updateLinksAfterRename(const std::string& oldTitle, const std::string& newTitle, RenameResult& result);
    
    /*quét thư mục đĩa, lọc các file .md
    trả về ds ghi chú thôi và các file bị bỏ qua
    */ 
    bool scanDisk(std::vector<Note>& out, std::vector<std::string>& skipped); 
public:
    explicit NoteManager(std::filesystem::path rootDir = "vault/notes");

    /*[[nodiscard]]*/ NoteError createNote(const std::string &title = "Untitled",
                     const std::string &content = "No content");

    /*[[nodiscard]]*/ NoteError validateNewTitle(const std::string& title) const;
    bool isTitleExists(const std::string& title) const;
    std::vector<NoteInfo> listNotes() const; 
    bool checkTitle(const std::string &input) const;


    /*[[nodiscard]]*/ LoadReport loadNotesFromDisk();    

    /*[[nodiscard]]*/ OpenResult openNote(const std::string &title);

    /*[[nodiscard]]*/ NoteError editNote(const std::string &title, const std::string &newContent);
    /*[[nodiscard]]*/ NoteError deleteNote(const std::string &title);
    /*[[nodiscard]]*/ RenameResult renameNote(const std::string& oldTitle, const std::string& newTitle);

    //search
    std::vector<SearchResult> searchContent(const std::string& keyword) const;
    std::vector<std::string> searchTitle(const std::string& keyword) const;


    //tag
    std::vector<std::string> listByTag(const std::string& tag) const;
    std::map<std::string, int> tagCounts() const;

    //backlinks
    std::vector<std::string> backlinks(const std::string& title) const;
    std::vector<BrokenLink> brokenLinks() const;


    SyncReport refreshFromDisk();
    void debugDump() const;   // in trang thai trong RAM (de debug)
};

#endif