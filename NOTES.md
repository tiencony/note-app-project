# Technical Debt & Known Issues (Nợ kỹ thuật & Lưu ý cần nâng cấp)

Danh sách các trường hợp edge-case, giới hạn thiết kế hiện tại và các điểm cần tối ưu trong tương lai.

## 1. Cảnh báo nguy cơ mất/lệch dữ liệu (Cần ưu tiên sửa)
* **`renameNote`**: Nếu đổi tên note sang tên `B` mà file `B.md` đã có trên đĩa nhưng RAM chưa cập nhật (ví dụ: người dùng tạo ngoài bằng Notepad), `std::filesystem::rename` có thể ghi đè làm mất dữ liệu file `B.md`.
  * *Hướng xử lý:* Kiểm tra `std::filesystem::exists` cho đường dẫn đích trước khi đổi tên (ngoại trừ trường hợp chỉ đổi hoa/thường của chính nó).
* **`editNote`**: Khi file bị sửa ngoài bằng Notepad/Explorer và tiếp tục sửa trong chương trình, chương trình sẽ ghi đè nội dung trên RAM lên đĩa (bản ghi sau thắng).
* **Đồng bộ RAM và Đĩa**: Các hàm chỉ đọc RAM (`searchTitle`, `searchContent`, `listByTag`, `tagCounts`, `backlinks`, `brokenLinks`, `listNotes`) dựa vào bản chụp (snapshot) lúc nạp, không theo dõi đĩa theo thời gian thực.
  * *Hướng xử lý:* Thêm tính năng "Reload from Disk" (`loadNotesFromDisk()`) hoặc cơ chế theo dõi file thay đổi (file watcher).

---

## 2. Các điểm tạm chấp nhận (Cần cải thiện sau)

| Hàm / Thành phần | Trường hợp hoạt động chưa tối ưu |
| :--- | :--- |
| **`listNotes`** | Kích thước/thời gian `lastModified` trong RAM có thể bị lệch so với đĩa khi file bị sửa ngoài hoặc sau khi gọi `renameNote`. |
| **`openNote`** | File bị xóa ngoài đĩa thì trả về `false`, nhưng thông tin note vẫn tồn tại trong RAM cho tới khi tải lại. |
| **`createNote`** | `create_directories` có thể ném exception nếu đường dẫn lỗi; nếu resize bộ nhớ thất bại thì file đã tạo trên đĩa nhưng RAM chưa ghi nhận. |
| **`editNote`** | Chưa kiểm tra lỗi đĩa đầy hoặc lỗi ghi file sau khi xuất stream. |
| **`loadNotesFromDisk`** | `directory_iterator` và `getFileTime` có thể ném exception nếu gặp lỗi hệ thống tập tin. |
| **`checkTitle`** | Chưa chặn các tên tệp dành riêng của hệ điều hành Windows (`CON`, `NUL`, `COM1`, ...); đang in lỗi trực tiếp ra stdout thay vì throw/return status code. |
| **`extractTags`** | Tag nằm trong khối code (`#tag`) hoặc tag chỉ chứa số (`#123`) vẫn bị nhận diện; chưa xử lý tag đứng liền sau ký tự đặc biệt như `,`, `"`. |
| **`extractLinks`** | Chưa hỗ trợ tách link dạng bí danh `[[Tên|Alias]]` hoặc link đến thẻ `[[Tên#Heading]]` (hiện tại coi toàn bộ chuỗi là tiêu đề note). |
| **`normalizeTag`** | Chưa cắt bỏ khoảng trắng thừa ở đầu và cuối chuỗi tag. |
| **`searchTitle`** | Từ khóa rỗng trả về toàn bộ danh sách note, trong khi `searchContent` từ khóa rỗng trả về kết quả rỗng (chưa thống nhất hành vi). |
| **Xu lý chuỗi & UTF-8** | Các hàm `equalsIgnoreCase`, `findIgnoreCase`, `makeSnippet` chỉ hoạt động chuẩn với ASCII; `makeSnippet` có thể cắt ngang ký tự multi-byte UTF-8. |
| **Thời gian hệ thống** | `std::localtime` không an toàn trong môi trường đa luồng (multi-threading); `last_write_time` có thể ném exception. |
| **Bộ test tự động** | Đang chạy trực tiếp trên thư mục `vault/notes` thật với hằng số `NOTES_DIR`, có thể gây lỗi test nhiễu nếu vault chứa ghi chú thật của người dùng. |







