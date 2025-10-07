#include "Logger.h"
#include <ctime>
#include <iomanip>
#include <sstream>
#include <iostream>
Logger::Logger(const std::string& filename) {
    logFile.open(filename, std::ios::app);
    if (!logFile.is_open()) {
        std::cerr << "Failed to open log file: " + filename << std::endl;
        throw std::runtime_error("Failed to open log file: " + filename);
    }
}

Logger& Logger::getInstance(const std::string& filename) {
    static Logger instance(filename);
    return instance;
}

void Logger::log(const std::string& message) {
    std::lock_guard<std::mutex> lock(logMutex);
    auto now = std::time(nullptr);
    std::stringstream ss;
    ss << "[" << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S") << "] " << message;
    logFile << ss.str() << std::endl;
    logFile.flush();
    std::cout << ss.str() << std::endl; // Вывод в терминал для отладки
}
