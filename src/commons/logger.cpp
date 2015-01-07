#include"logger.h"
#include"filewriter.h"

void Logger::write(std::string filePath, std::string message)
{
    FileWriter fw;
    fw.write(filePath, message);
}

void Logger::writeLog(std::string logMessage)
{
    write("logs/status.log", logMessage);
}

void Logger::writeError(std::string errorMessage)
{
    write("logs/error.log", errorMessage);
}

