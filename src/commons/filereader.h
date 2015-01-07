#ifndef FILEREADER_H
#define FILEREADER_H

#include<fstream>
#include<string>
#include<vector>

class FileReader
{
    private:
        std::ifstream file;

    public:
        FileReader();
        FileReader(std::string);
        ~FileReader();
        std::string read();
        std::vector<std::string> read(std::string);
        static std::vector<std::string> getFiles(std::string, std::string);
};

#endif
