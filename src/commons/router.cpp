#include<cstdlib>
#include<string>
#include<vector>
#include<sstream>
#include"router.h"
#include"filewriter.h"
#include"lsa.h"
#include"constants.h"
#include"logger.h"

using namespace std;

/*Class: Router implementation*/

/*Router: private methods*/
void Router::parseIP(int argc, char **argv, int index)
{
    while(index < argc)
    {
        IPAddress ipAddr;
        ipAddr.setNetNum(string(*(argv + index)));
        index++;
        ipAddr.setHostNum(string(*(argv + index)));
        index++;
        this->ipAddresses.push_back(ipAddr);
    }
    this->outFileName = FILE_PREFIX_OUT + this->id + FILE_EXT_TXT;
    this->rtFileName = FILE_PREFIX_RT + this->id + FILE_EXT_TXT;
}

void Router::clearFileReaders()
{
    /*free memory used by FileReader pointers*/
    for(vector<FileReader*>::iterator it=this->readers.begin(); 
        it!=this->readers.end(); ++it)
    {
        delete *it;
    }
    this->readers.clear();
}

void Router::clearRouterCache()
{
    for(map<string, RouterCache*>::iterator it=this->iRouters.begin(); 
        it!=this->iRouters.end(); ++it)
    {
        delete it->second;
    }
    this->iRouters.clear();
}

bool Router::verifyAndUpdateTopology(string lsa)
{
    return updateRouterCache(iRouters, lsa);
}

bool Router::updateRouterCache(map<string, RouterCache*>& cache, string lsa)
{    
    map<string, RouterCache*>::iterator it;
    int bcastId = atoi(LSA::getBcastId(lsa).c_str());
    string routerId = LSA::getRouterId(lsa);
    it = cache.find(routerId);
    if(it != cache.end())
    {
        it->second->resetHoldingTime();
        if(bcastId > it->second->getBcastId())
        {
            it->second->setBcastId(bcastId);
            it->second->setLSA(lsa);
            return true;
        }
        else
            return false;
    }
    RouterCache *rc = new RouterCache();
    rc->setBcastId(bcastId);
    rc->setLSA(lsa);
    rc->resetHoldingTime();
    cache[routerId] = rc;
    return true;
}

void Router::forwardLSA(string lsa)
{
    string iifNetNum = LSA::getDestNetNum(lsa);
    for(vector<IPAddress>::iterator it=this->ipAddresses.begin();
        it!=this->ipAddresses.end(); ++it)
    {
        string currNetNum = it->getNetNum();
        if(currNetNum == iifNetNum)
            continue;
        lsa.replace(9, 2, currNetNum);
        FileWriter fw(this->outFileName);
        fw.write(lsa);
    }
}

string Router::buildLSA(IPAddress& ip, string networks)
{
    string str;
    str.reserve(LSA_MIN_LENGTH + networks.size());
    str.append(ip.toString(false));
    str.append(" ");
    str.append(ip.toString(true));
    str.append(" ");
    str.append(MESSAGE_TYPE_LSA);
    str.append(" ");
    str.append(this->id);
    str.append(" ");
    stringstream bcastId;
    if(this->broadcastId < 10)
        bcastId << 0;
    bcastId << this->broadcastId;
    str.append(bcastId.str());
    str.append(" ");
    str.append(MESSAGE_TYPE_NETWKS);
    str.append(networks);
    return str;        
}

void Router::buildTempList(map<string, RouteEntry>::iterator confRE, map<string, RouteEntry>& tempList,
                    map<string, RouteEntry> confList)
{
    for(map<string, RouterCache*>::iterator it=this->iRouters.begin();
        it!=this->iRouters.end(); ++it)
    {
        set<string> netwks = LSA::getNetwks(it->second->getLSA());
        this->buildTempList(confRE, tempList, confList, it->second->getLSA(), netwks);
    }
}

void Router::buildTempList(map<string, RouteEntry>::iterator confRE, map<string, RouteEntry>& tempList,
                    map<string, RouteEntry> confList, string lsa, set<string> netwks)
{
        if(netwks.find(confRE->first) == netwks.end())                 //not an adjacent router
            return;
        for(set<string>::iterator netIt=netwks.begin(); netIt!=netwks.end(); ++netIt)
        {
            if(confList.find(*netIt) != confList.end())       //already in confList
                continue;
            map<string, RouteEntry>::iterator tempRE = tempList.find(*netIt);
            if(tempRE != tempList.end())
            {
                if((confRE->second).cost + 1 < (tempRE->second).cost)
                {
                    tempList[*netIt] = createNewRouteEntry(confRE, lsa);
                }
            }
            else
            {
                tempList[*netIt] = createNewRouteEntry(confRE, lsa);
            }
        }
}

RouteEntry Router::createNewRouteEntry(map<string, RouteEntry>::iterator confRE,
    string lsa)
{
    RouteEntry re;
    re.cost = (confRE->second).cost + 1;
    if((confRE->second).ip.getNetNum() == IP_NA)
    {
        IPAddress ip(LSA::getSrcNetNum(lsa), LSA::getSrcHostNum(lsa));
        re.ip = ip;
    }
    else
        re.ip = (confRE->second).ip;
    return re;
}

string Router::getNextConfRouteEntryKey(map<string, RouteEntry> tempList)
{
    int leastCost = 1000;
    for(map<string, RouteEntry>::iterator it=tempList.begin(); it!=tempList.end(); ++it)
    {
        if((it->second).cost < leastCost)
            leastCost = (it->second).cost;
    }
    map<string, RouteEntry> leastCostList;
    for(map<string, RouteEntry>::iterator it=tempList.begin(); it!=tempList.end(); ++it)
    {
        if((it->second).cost == leastCost)
            leastCostList[it->first] = it->second;
    }
    return leastCostList.begin()->first;
}

set<string> Router::getDeadRouters(map<string, RouterCache*>& routers)
{
    /*Reduce holding times of each entry by 1*/
    set<string> deadRouters;
    for(map<string, RouterCache*>::iterator it=routers.begin();
        it!=routers.end(); ++it)
    {
        it->second->decrementHoldingTime();
        if(it->second->getHoldingTime() <= 0)
        {
            deadRouters.insert(it->first);
        }
    }
    return deadRouters;
}

/*Router: public methods*/
Router::Router()
{   
    this->broadcastId = 0;
}

Router::~Router()
{
    this->clearFileReaders();
    this->clearRouterCache();
}

void Router::parse(int argc, char **argv)
{
    //TODO: Handle errors
    int i = 1;
    this->id = string(*(argv + i));
    i++;
    this->ipAddresses.reserve((argc-2)/2);
    this->parseIP(argc, argv, i);
}

string Router::getId()
{
    return this->id;
}

vector<IPAddress> Router::getIpAddresses()
{
    return this->ipAddresses;
}

int Router::getBroadcastId()
{
    return this->broadcastId;
}

void Router::broadcastLSA()
{
    FileWriter fw(this->outFileName);
    this->broadcastId++;     
    string networks;
    networks.reserve(this->ipAddresses.size()*3);
    for(vector<IPAddress>::iterator it=this->ipAddresses.begin();
        it!=this->ipAddresses.end(); ++it)
    {
        networks.append(" ");
        networks.append(it->getNetNum());
    }
    for(vector<IPAddress>::iterator it=this->ipAddresses.begin();
        it!=this->ipAddresses.end(); ++it)
    {
        string lsaStr = this->buildLSA(*it, networks);
        fw.write(lsaStr);
    }
}

set<string> Router::readNetwks()
{
    set<string> directAddressMessages;
    if(this->ipAddresses.size() == 0)
        return directAddressMessages;
    if(this->readers.size() == 0)
    {
        try
        {
            for(vector<IPAddress>::iterator it=this->ipAddresses.begin();
                it!=this->ipAddresses.end(); ++it)
            {
                FileReader *reader = new FileReader(FILE_PREFIX_NET + it->getNetNum() + FILE_EXT_TXT);
                this->readers.push_back(reader);
            }
        }
        catch(...)
        {
            //TODO:Files not available yet
            this->clearFileReaders();
        }
    }

    for(vector<FileReader*>::iterator it=this->readers.begin(); 
        it!=this->readers.end(); ++it)
    {
        while(true)
        {
            string msg = (*it)->read();
            if(msg.size() == 0)
                break;
            if(LSA::getRouterId(msg) != this->id)
            {
                if(LSA::getDestHostNum(msg) == BCAST_IP)
                {
                    if(this->verifyAndUpdateTopology(msg))        //if new, verifyAndUpdateTopology method updates topology
                        this->forwardLSA(msg);
                }
                else   //its a BGP message                  
                {
                    directAddressMessages.insert(msg);
                }
            }
        }
    }

    set<string> deadRouters = this->getDeadRouters(iRouters);
    for(set<string>::iterator it=deadRouters.begin(); it!=deadRouters.end(); ++it)
    {
        iRouters.erase(*it);
    }

    return directAddressMessages; 
}

/*Compute and write*/
void Router::writeRoutingTable(string time)
{
    map<string, RouteEntry> confList; 
    map<string, RouteEntry> tempList;
    
    //add directly connected networks to confirmed list
    for(vector<IPAddress>::iterator it=this->ipAddresses.begin();
        it!=this->ipAddresses.end(); ++it)
    {
        RouteEntry re;
        re.cost = 0;
        re.ip.setNetNum(IP_NA);
        re.ip.setHostNum(IP_NA);
        confList[it->getNetNum()] = re;
    }
    for(map<string, RouteEntry>::iterator it=confList.begin(); it!=confList.end(); ++it)
    {
        this->buildTempList(it, tempList, confList);
    }    
    
    while(tempList.size() > 0)
    {
        string nextConfEntryKey = this->getNextConfRouteEntryKey(tempList);
        confList[nextConfEntryKey] = tempList[nextConfEntryKey];
        tempList.erase(nextConfEntryKey);
        this->buildTempList(confList.find(nextConfEntryKey), tempList, confList);
    }
     
    FileWriter fw(this->rtFileName);
    
    this->intradomainNetwks.clear();
    for(map<string, RouteEntry>::iterator it=confList.begin(); it!=confList.end(); ++it)
    {
        fw.write(time + " " + it->first + " " + (it->second).ip.toString(false));
        this->intradomainNetwks.insert(it->first);
    } 
    this->routingTable = confList;
}

