# Program Architecture & Execution Flows (Sơ đồ luồng hệ thống)

Tài liệu này phác thảo các luồng hoạt động chính của project C++, giúp hình dung cách thức tương tác giữa các thành phần (Giao diện menu, Quản lý dữ liệu trên RAM, Tệp trên Đĩa và Bộ nhớ đệm chỉ mục liên kết).

---

## 1. Bức tranh tổng thể: Ai gọi ai
'''text

┌─────────────────────────────────────────────┐
│  main.cpp   Giao diện (menu) + Bộ test      │
└───────────────────┬─────────────────────────┘
                    │ gọi
                    ▼
┌─────────────────────────────────────────────┐
│  NoteManager   "Người quản lý"              │
│   · vector<Note> notesArray       [RAM]     │
│   · Chỉ mục link (3 unordered_map)[RAM]     │
└───────┬───────────────────────┬─────────────┘
        │ chứa                  │ dùng
        ▼                       ▼
┌───────────────┐      ┌────────────────────────────┐
│ Note          │      │ Utils  Hàm "thuần"         │
│ title         │      │  Chuỗi: equalsIgnoreCase,  │
│ content       │      │         findIgnoreCase,    │
│ lastModified  │      │         makeSnippet        │
└───────────────┘      │  Markdown: extractTags,    │
                       │   extractLinks, normalize* │
                       │  Thời gian, readFileToString│
                       └────────────────────────────┘

       NoteManager đọc/ghi         ┌──────────────────┐
       ───────────────────────────►│ [DISK]           │
                                   │ vault/notes/*.md │
                                   └──────────────────┘


## 2. Chương trình chạy từ đầu đến cuối
main(argc, argv)
 │
 ├─ argv[1] == "--test" ?
 │     ├─ CÓ  ──► runAutoTest() ──► Trả về 0 (PASS hết) hoặc 1
 │     │          cleanupTestFiles → testLinks → testTags → ... → In kết quả
 │     │
 │     └─ KHÔNG ► runMenu()
 │                 │
 │                 ├─ NoteManager manager
 │                 ├─ manager.loadNotesFromDisk()   [DISK ──► RAM] (1 lần khởi tạo)
 │                 │
 │                 └─ Vòng lặp while(choice != 0):
 │                       In menu → Đọc lựa chọn
 │                       ├─ Nhập sai định dạng → Báo lỗi, quay lại
 │                       └─ switch(choice)
 │                            1  Tạo mới ────► createNote        [RAM + DISK]
 │                            2  Liệt kê ───► listNotes         [RAM]
 │                            3  Tìm title ─► searchTitle       [RAM]
 │                            4  Tìm nội dung► searchContent   [RAM]
 │                            5  Mở note ───► openNote          [DISK đọc lại 1 note]
 │                            6  Sửa note ──► editNote          [RAM + DISK]
 │                            7  Đổi tên ───► renameNote        [RAM + DISK]
 │                            8  Xóa note ───► deleteNote       [RAM + DISK]
 │                            9  Theo tag ──► listByTag         [RAM]
 │                            10 Đếm tag ───► tagCounts         [RAM]
 │                            11 Backlinks ─► backlinks         [RAM, qua chỉ mục]
 │                            0  Thoát
 │
 └─ return 0


## 3. Một thao tác cụ thể: createNote

 createNote(title, content)
 │
 ├─1 checkTitle(title)          Rỗng / Ký tự cấm / Kết thúc '.' hoặc ' '? ──► false
 ├─2 isTitleExists(title)  [RAM]  Đã có trong vector chưa? ─────────────────► false
 ├─3 fs::exists(path)      [DISK] File đã nằm trên đĩa từ trước? ───────────► false
 │       (Phòng tránh ghi đè file được tạo ngoài chương trình)
 ├─4 ofstream ghi file     [DISK] Mở hoặc ghi file thất bại? ───────────────► false
 ├─5 notesArray.push_back  [RAM]
 ├─6 linkIndexDirty = true      Đánh dấu: "Chỉ mục link bây giờ đã cũ (dirty)"
 └─► true


## 4. Chỉ mục link: ba bảng hash và cách chúng được làm mới

 createNote / editNote / renameNote / deleteNote / loadNotesFromDisk
        │
        └──► linkIndexDirty = true     (Chỉ bật cờ, thao tác cực kỳ nhanh)

 backlinks() / brokenLinks()
        │
        ▼
   ensureLinkIndex()
        ├─ dirty == true  ──► rebuildLinkIndex() Duyệt mọi note, gọi extractLinks
        │                      linkIndexDirty = false   (Tốn chi phí O(tổng số link))
        └─ dirty == false ──► Không làm gì (Bỏ qua bước tính toán lại)
        │
        ▼
   Trả kết quả tra cứu hash (O(1) cho mỗi lần tra)


## 5. Luồng xử lý khi file bị chỉnh sửa bên ngoài (Notepad/Explorer)

THỜI GIAN ───────────────────────────────────────────────────────────────────►

 Chương trình: loadNotesFromDisk()      [DISK ──► RAM]  (RAM lưu bản chụp lúc này)
 Người dùng:    Mở Notepad, thêm "#cpp" vào file a.md, lưu lại [DISK đổi, RAM chưa biết]
 Chương trình: listByTag("cpp")         [RAM] ──► Không tìm thấy  ✗  (Dữ liệu cũ)
 Chương trình: openNote("a")            [DISK] Đọc lại file a.md ──► Thấy "#cpp" ✓
                                         (RAM của riêng note "a" được cập nhật)
 Chương trình: listByTag("cpp")         [RAM] ──► Giờ mới thấy kết quả ✓
