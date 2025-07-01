    #ifndef LOGGER_H
#define LOGGER_H
#include <fstream>
#include <string>
#include <mutex>

class Logger {
private:
  std::mutex mtx;
    std::ofstream file;
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

public:
    static Logger& getInstance();
    void log(const std::string& message);

};

#endif
