#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <cstddef>
#include <string>

bool equalsIgnoreCase(const std::string& str1, const std::string& str2);
std::size_t findIgnoreCase(const std::string& text, const std::string& keyword);
bool containsIgnoreCase(const std::string& text, const std::string& keyword);

std::string makeSnippet(const std::string& text, std::size_t pos, std::size_t keywordLen);

std::string trimSpaces(const std::string& s);

#endif