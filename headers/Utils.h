#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <filesystem>
#include <set>
#include <ctime> 


std::string getCurrentTime();
std::string getFileTime(const std::filesystem::path& p);
std::string formatTime(std::time_t t);


std::size_t findIgnoreCase(const std::string& text, const std::string& keyword);
bool containsIgnoreCase(const std::string& text, const std::string& keyword);
bool equalsIgnoreCase(const std::string& str1, const std::string& str2);

std::string makeSnippet(const std::string& text, std::size_t pos, std::size_t keywordLen);


// Doc nd tu disk va luu vao 1 string
bool readFileToString(const std::filesystem::path& path, std::string& out);

std::set<std::string> extractTags(const std::string& content);
std::string normalizeTag(const std::string& tag);
std::set<std::string> extractLinks(const std::string& content);

#endif