#include "../headers/Utils.h"
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <cctype>
#include <algorithm>


std::string getCurrentTime(){
    std::time_t now = std::time(nullptr);
    std::tm* local = std::localtime(&now);
    std::ostringstream oss;
    oss << std::put_time(local, "%Y-%m-%d %H:%M:%S");
    return oss.str();
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