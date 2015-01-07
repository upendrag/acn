#ifndef LOGGER_H
#define LOGGER_H

#include<string>

class Logger
{
    private:
        static void write(std::string, std::string); 
    public:
        static void writeLog(std::string);
        static void writeError(std::string);
};

#endif
