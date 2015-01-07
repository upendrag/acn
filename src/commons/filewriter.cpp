#include"filewriter.h"
#include"constants.h"

using namespace std;

FileWriter::FileWriter(string fileName)
{
    string path = FILES_DIR + fileName;
                
    file.open(path.c_str(), ios_base::out | ios_base::app);
}

FileWriter::FileWriter() {}

FileWriter::~FileWriter()
{
    if(file.is_open())
        file.close();
}

bool FileWriter::write(string data)
{
    file << data << endl;
    return true;
}

bool FileWriter::write(string filePath, string data)
{
    file.open(filePath.c_str(), ios_base::out | ios_base::app);
    file << data << endl;
    file.close();
    return true;
}
