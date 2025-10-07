#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <mutex>

class Logger {
public:
    static Logger& getInstance(const std::string& filename = "log.txt");
    void log(const std::string& message);
private:
    Logger(const std::string& filename);
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    std::ofstream logFile;
    std::mutex logMutex;
};

#endif
