#include <fstream>
#include <string>
#include <ctime>
#include <iomanip>
#include <sstream>
#include "Logger.h"
#include <mutex>

    std::mutex mtx;
    std::ofstream file;
    Logger::Logger()  : file("log.txt",std::ios::app)
    {
        if(!file.is_open())
        {
            throw std::runtime_error("Failed to open log.txt");
        }
    };
  Logger&   Logger:: getInstance()
    {
        static Logger instance;
        return instance;
    }

    void  Logger:: log(const std::string& message)
    {
        std::lock_guard<std::mutex>lock(mtx);
        std::time_t currentTime= std::time(nullptr);
        std::tm* localTime = std::localtime(&currentTime);
        std::stringstream ss;
        ss<<std::put_time(localTime,"%Y-%m-%d %H:%M:%S");
        std::string timestamp=ss.str();

        file<<" [" << timestamp<< "] "<<message<<"\n";


    }
      Logger::~Logger()
    {
       file.close();
    }
