#include"ipaddress.h"
#include"constants.h"

using namespace std;

/*Class: IPAddress implementation*/
string IPAddress::getNetNum()
{
    return this->netNum;
}

void IPAddress::setNetNum(string num)
{
    this->netNum = num;
}
    
string IPAddress::getHostNum()
{
    return this->hostNum;
}

void IPAddress::setHostNum(string num)
{
    this->hostNum = num;
}

string IPAddress::toString(bool isBcast)
{
    string ip;
    ip.reserve(IP_STR_LEN);
    ip.append("(");
    ip.append(this->netNum);
    ip.append(",");
    if(isBcast)
        ip.append(BCAST_IP);
    else
        ip.append(this->hostNum);
    ip.append(")");
    return ip;
}

IPAddress::IPAddress(){}

IPAddress::IPAddress(const IPAddress& ipAddr)
{
    netNum = ipAddr.netNum;
    hostNum = ipAddr.hostNum;
}

IPAddress::~IPAddress(){}

IPAddress::IPAddress(string nNum, string hNum): netNum(nNum), hostNum(hNum)
{}

