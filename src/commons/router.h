#ifndef ROUTER_H
#define ROUTER_H

#include<string>
#include<vector>
#include<map>
#include<set>
#include"ipaddress.h"
#include"routercache.h"
#include"filereader.h"

class RouteEntry
{
    public:
        int cost;
        IPAddress ip;
};

class Router
{
    private:
        int broadcastId;
        std::vector<FileReader*> readers;

        void clearFileReaders();
        void clearRouterCache();
        virtual void buildTempList(std::map<std::string, RouteEntry>::iterator, std::map<std::string, RouteEntry>&,
            std::map<std::string, RouteEntry>);
        RouteEntry createNewRouteEntry(std::map<std::string, RouteEntry>::iterator, std::string);
        std::string getNextConfRouteEntryKey(std::map<std::string, RouteEntry>);

    protected:
        std::string id;
        std::vector<IPAddress> ipAddresses;
        std::string outFileName;
        std::string rtFileName;
        std::map<std::string, RouterCache*> iRouters;
        std::set<std::string> intradomainNetwks;
        std::map<std::string, RouteEntry> routingTable;

        void parseIP(int, char**, int);
        virtual std::string buildLSA(IPAddress&, std::string);
        virtual bool verifyAndUpdateTopology(std::string);
        virtual void forwardLSA(std::string);
        bool updateRouterCache(std::map<std::string, RouterCache*>&, std::string);
        void buildTempList(std::map<std::string, RouteEntry>::iterator, std::map<std::string, RouteEntry>&,
            std::map<std::string, RouteEntry>, std::string, std::set<std::string>);
        std::set<std::string> getDeadRouters(std::map<std::string, RouterCache*>&);

    public:
        Router();
        virtual ~Router();
        virtual void parse(int, char**);
        std::string getId();
        std::vector<IPAddress> getIpAddresses();
        int getBroadcastId();
        void broadcastLSA();
        virtual std::set<std::string> readNetwks();
        virtual void writeRoutingTable(std::string);
};

#endif
