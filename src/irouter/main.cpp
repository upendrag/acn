#include<iostream>
#include<unistd.h>
#include<sstream>
#include<router.h>

using namespace std;

int main(int argc, char *argv[])
{
    Router *irouter = new Router();
    irouter->parse(argc, argv);
    int i = 0;
    while(i<100)
    {
        if(i % 10 == 0)
        {
            irouter->broadcastLSA();
        }

        if(i != 0 && i % 15 == 0)
        {
            stringstream ss;
            ss << i;
            irouter->writeRoutingTable(ss.str());
        }
        
        usleep(1*1000*1000);
        
        irouter->readNetwks();
        

        i++;
    }

    delete irouter;
    return 0;
}
