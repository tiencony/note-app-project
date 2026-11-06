#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <filesystem>

std::string getCurrentTime();
bool containsIgnoreCase(const std::string& text, const std::string& keyword);
bool equalsIgnoreCase(const std::string& str1, const std::string& str2);
std::string getFileTime(const std::filesystem::path& p);

#endif