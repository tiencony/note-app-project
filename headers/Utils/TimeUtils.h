#ifndef TIME_UTILS_H
#define TIME_UTILS_H

#include <ctime>
#include <filesystem>
#include <string>

std::string formatTime(std::time_t t);
std::string getCurrentTime();
std::string getFileTime(const std::filesystem::path& p);

#endif