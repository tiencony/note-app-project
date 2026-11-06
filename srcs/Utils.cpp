#include "../headers/Utils.h"
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <cctype>
#include <algorithm>
#include <filesystem>


namespace fs = std::filesystem;

// Tham khao phan Time
std::string formatTime(std::time_t t){
    std::tm* local = std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(local, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string getCurrentTime(){
    return formatTime(std::time(nullptr));
}

std::string getFileTime(const fs::path& p){
    auto ftime = fs::last_write_time(p);
    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
    return formatTime(std::chrono::system_clock::to_time_t(sctp));
}


bool containsIgnoreCase(const std::string& text, const std::string& keyword){
    auto it = std::search(
        text.begin(), text.end(),
        keyword.begin(), keyword.end(),
        [](unsigned char a, unsigned char b){
            return std::tolower(a) == std::tolower(b);
        }
    );
    return it != text.end();
}

bool equalsIgnoreCase(const std::string& str1, const std::string& str2){
    if(str1.length() != str2.length()) return false;

    return std::equal(
        str1.begin(), str1.end(),
        str2.begin(), str2.end(),
        [](unsigned char ch1, unsigned char ch2){
            return std::tolower(ch1) == std::tolower(ch2);
        }
    );

}


