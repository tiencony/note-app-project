// StringUtils.h - Tien ich chuoi nho, khong phu thuoc module nao khac.
// Hoa/thuong chi dung cho ASCII; trimSpaces chi cat ' ' va '\t'.
// trimSpaces tra string_view TRO VAO chuoi goi: vi vay ban nhan std::string&&
// bi "= delete" de chan trim mot temporary. Ket qua khong duoc song lau hon
// chuoi dau vao.


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
std::string_view trimSpaces(const std::string&&) = delete;

// Chuoi literal "..." van dung binh thuong (song suot chuong trinh)
inline std::string_view trimSpaces(const char* s){return trimSpaces(std::string_view(s)); }
#endif