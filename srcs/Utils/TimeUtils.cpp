#include "Utils/TimeUtils.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <system_error>

namespace fs = std::filesystem;

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
    std::error_code ec;
    auto ftime = fs::last_write_time(p, ec);
    if(ec) return "Unknown";

    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
    return formatTime(std::chrono::system_clock::to_time_t(sctp));
}