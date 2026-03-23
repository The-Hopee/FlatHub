#ifndef LOGGER_HPP_
#define LOGGER_HPP_
#include <memory>
#include <mutex>
#include <fstream>

class Logger
{
public:
    static Logger& Instance()
    {
        static Logger inst;
        return inst;
    }

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;

    // Виды логов
    void info(const std::string_view component, const std::string& msg) { log(component,"INFO", msg); }
    void error(const std::string_view component, const std::string& msg) { log(component,"ERROR", msg); }
    void debug(const std::string_view component, const std::string& msg) { log(component,"DEBUG", msg); }
private:
    Logger()
    {
        log_file.open("../logs/server.log");
    }
    ~Logger()
    {
        if(log_file.is_open()) log_file.close();
    }

    std::mutex m;
    std::ofstream log_file;

    void log(const std::string_view, const std::string_view, const std::string&);
};

#endif