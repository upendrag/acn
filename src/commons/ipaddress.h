#ifndef IPADDRESS_H
#define IPADDRESS_H

#include<string>

class IPAddress
{
    private:
        std::string netNum;
        std::string hostNum;
            
    public:
        std::string getNetNum();
        void setNetNum(std::string);
        std::string getHostNum();
        void setHostNum(std::string);
        std::string toString(bool);
        IPAddress();
        IPAddress(const IPAddress&);
        ~IPAddress();
        IPAddress(std::string, std::string);
};

#endif
