#include<sys/types.h>
#include<dirent.h>
#include"filereader.h"
#include"filewriter.h"
#include"logger.h"
#include"constants.h"

using namespace std;

FileReader::FileReader(){}

FileReader::FileReader(string fileName)
{
    file.open((FILES_DIR + fileName).c_str(), ios::in);    
}

FileReader::~FileReader()
{
    if(file.is_open())
        file.close();
}

string FileReader::read()
{
    string msg;
    if(file.eof())
        file.clear();    
    getline(file, msg);
    return msg;
}

vector<string> FileReader::read(string filePath)
{
    file.open(filePath.c_str(), ios::in);
    vector<string> lines;
    while(!file.eof())
    {
        string line;
        getline(file, line);
        if(line.size() > 0)
        {
            lines.push_back(line);
        }
    }
    file.close();
    return lines;
}

vector<string> FileReader::getFiles(string dirPath, string prefix)
{
    DIR *dir;
    struct dirent *de;
    vector<string> files;
    
    dir = opendir(dirPath.c_str());
    if(dir != NULL)
    {
        while((de = readdir(dir)) != NULL)
        {
            string fileName(de->d_name);
            if(fileName.substr(0,3) == FILE_PREFIX_OUT)
                files.push_back(fileName);
        }
        (void) closedir(dir);
    }
    else
    {
        //TODO:error reporting
    }
    return files;
}
