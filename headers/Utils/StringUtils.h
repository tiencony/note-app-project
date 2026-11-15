#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <cstddef>
#include <string>
#include <vector>
#include <string_view>



bool equalsIgnoreCase(std::string_view str1, std::string_view str2);

std::size_t findIgnoreCase(std::string_view text, std::string_view keyword);

bool containsIgnoreCase(std::string_view text, std::string_view keyword);

std::string makeSnippet(std::string_view text, std::size_t pos, std::size_t keywordLen);

std::string_view trimSpaces(std::string_view s);

#endif