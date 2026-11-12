#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <filesystem>
#include <string>

// Doc toan bo noi dung file vao mot string
bool readFileToString(const std::filesystem::path& path, std::string& out);
bool writeFileAtomic(const std::filesystem::path& path, const std::string& content);


#endif