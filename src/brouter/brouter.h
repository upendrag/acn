#ifndef BROUTER_H
#define BROUTER_H

#include<utility>
#include<router.h>
#include"bgppath.h"

class BRouter: public Router
{
    private:
        std::string asId;
        std::map<std::string, RouterCache*> iBGPpeers;
        std::map<std::string, RouterCache*> eBGPpeers;
        std::set<std::string> sentBGPmessages;
        std::set<std::string> customers;
        std::set<std::string> providers;
        std::set<std::string> peers;
        std::vector<BGPPath> cachedPaths;
        std::map<std::string, BGPPath> preferredPaths;
        std::map<std::string, std::set<std::pair<std::string, std::string> > > exportedPaths;

        bool isAttachedToMyNetwks(std::string);
        std::string buildLSA(IPAddress&, std::string);
        std::string getNetwksToInject();
        bool verifyAndUpdateTopology(std::string);
        void handleDeadRouters();
        void forwardLSA(std::string);
        void buildTempList(std::map<std::string, RouteEntry>::iterator, std::map<std::string, RouteEntry>&,
            std::map<std::string, RouteEntry>);
        void removeInjectedNetworks(std::set<std::string>&, std::set<std::string>);
        bool isNewPathToNetwkBetter(BGPPath, BGPPath);
        void writeBGPmsg(std::map<std::string, RouterCache*>, std::string);
        void readHBGP();
        void processBGPmessage(std::string);
        bool setPreferredPath(BGPPath);
        bool isABetterPath(BGPPath, BGPPath);
        bool isAnUpdateMessage(BGPPath);
        bool hasLeastInternalCost(BGPPath, BGPPath);
        int getInternalCostToRouter(std::string);
        void exportPathToInternalPeers(std::string, BGPPath);
        void exportPathToProviders(std::string, BGPPath);
        void exportPathToPeers(std::string, BGPPath);
        void exportPathToCustomers(std::string, BGPPath);
        void exportPathToOtherDomains(std::string, std::set<std::string>, BGPPath);
        void removeFromCachedPaths(std::set<std::string>);
        void withdrawRoutes(std::set<std::string>);
        BGPPath getAltPreferredPath(std::string);
        void sendWithdrawMessage(std::string, std::string);
        void sendAltBGPPath(std::string, BGPPath);
        std::string getSrcIPforBGP(std::string, bool);
        void handleWithdrawMsg(std::string);

    public:
        void parse(int, char**);
        std::set<std::string> readNetwks();
        void writeRoutingTable(std::string);
};

#endif
