#ifndef FILEWRITER_H
#define FILEWRITER_H

#include<fstream>
#include<string>

class FileWriter
{
    private:
        std::fstream file;
    public:
        FileWriter();
        FileWriter(std::string);
        ~FileWriter();
        bool write(std::string);
        bool write(std::string, std::string);
};

#endif
