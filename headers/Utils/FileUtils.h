#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <filesystem>
#include <string>

// Bỏ BOM UTF-8; "\r\n" và "\r" đơn lẻ -> "\n".
std::string normalizeContent(std::string_view s);

// Đọc file (binary), rồi chuẩn hóa. false nếu không mở/đọc được.
bool readFileToString(const std::filesystem::path& path, std::string& out);

// Ghi `content` (ghi nguyên văn, KHÔNG chuẩn hóa: người gọi phải làm trước).
// overwrite = false: thất bại nếu file đích đã tồn tại.
bool writeFileAtomic(const std::filesystem::path& path,
                     const std::string& content,
                     bool overwrite = true);

#endif