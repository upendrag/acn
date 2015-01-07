//#include<iostream>
#include<unistd.h>
#include<vector>
#include<filewriter.h>
#include<filereader.h>
#include<lsa.h>
#include<constants.h>

using namespace std;

void writeMessagetoNetwk(string);

int main(void)
{
    int timer=0;
    usleep(250*1000);
    
    vector<string> outFiles = FileReader::getFiles(FILES_DIR, "out");
    vector<FileReader*> readers; 
    
    for(vector<string>::iterator it=outFiles.begin(); it != outFiles.end(); ++it)
    {
        FileReader *reader = new FileReader(*it);
        readers.push_back(reader);  
    }
    
    while(timer<200)
    {
        for(vector<FileReader*>::iterator it=readers.begin(); it != readers.end(); ++it)
        {
            while(true)
            {
                string msg = (*it)->read();
                if(msg.size() == 0)
                    break;
                writeMessagetoNetwk(msg);
            }
        }             
        
        usleep(500*1000);       
        timer++;
    }

    /*Free memory used by FileReaders*/
    for(vector<FileReader*>::iterator it=readers.begin(); it != readers.end(); ++it)
    {
        delete *it;
    }
    readers.clear();             

    return 0;
}

void writeMessagetoNetwk(string msg)
{
    string netNum = LSA::getDestNetNum(msg);
    FileWriter fw(FILE_PREFIX_NET + netNum + FILE_EXT_TXT);
    fw.write(msg);
}
