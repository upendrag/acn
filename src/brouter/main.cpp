#include<unistd.h>
#include<sstream>
#include"brouter.h"

using namespace std;

int main(int argc, char *argv[])
{
    Router *brouter = new BRouter();
    brouter->parse(argc, argv);
    int i = 0;
    while(i<100)
    {
        if(i % 10 == 0)
        {
            brouter->broadcastLSA();
        }

        if(i != 0 && i % 15 == 0)
        {
            stringstream ss;
            ss << i;
            brouter->writeRoutingTable(ss.str());
        }
        
        usleep(1*1000*1000);
        
        brouter->readNetwks();
        

        i++;
    }

    delete brouter;
    return 0;
}
