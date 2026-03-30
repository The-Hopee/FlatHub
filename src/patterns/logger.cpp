#include "../../include/logger.hpp"
#include <chrono>
#include <iostream>
#include <iomanip>

void Logger::log(const std::string& log_from, const std::string& type_log, const std::string& log_msg)
{
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);

    std::lock_guard<std::mutex> lock(m);

    log_file << "[" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << "] "
                <<"[" << log_from << "] "
                << "[" << type_log << "] "
                << log_msg << '\n';

    //if (type_log == "ERROR") {
    log_file.flush();
    //}
        
    std::cout << "[" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << "] "
                <<"[" << log_from << "] "
                << "[" << type_log << "] "
                << log_msg << std::endl;
}