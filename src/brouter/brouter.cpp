#include<cstdlib>
#include<set>
#include<sstream>
#include<logger.h>
#include<constants.h>
#include<lsa.h>
#include<filewriter.h>
#include<filereader.h>
#include"brouter.h"
#include"bgp.h"
#include"bgppath.h"

using namespace std;

bool BRouter::isAttachedToMyNetwks(string netNum)
{
    for(vector<IPAddress>::iterator it=this->ipAddresses.begin(); 
        it!=this->ipAddresses.end(); ++it)
    {
        if(it->getNetNum() == netNum)
            return true;
    }
    return false;
}

void BRouter::parse(int argc, char **argv)
{
   int i = 1;
   this->id = string(*(argv + i));
   i++;
   this->asId = string(*(argv + i));
   i++;
   this->ipAddresses.reserve((argc-3)/2);
   this->parseIP(argc, argv, i);
   this->readHBGP();
}

string BRouter::buildLSA(IPAddress& ipAddr, string networks)
{
    string lsaB = Router::buildLSA(ipAddr, networks); //initial part of LSA message is same
    string netwksToInject = this->getNetwksToInject();
    
    if(!netwksToInject.empty())
    {
        lsaB.append(netwksToInject);
    }
    lsaB.append(" ");
    lsaB.append(MESSAGE_TYPE_OPTIONS);
    lsaB.append(" ");
    lsaB.append(MESSAGE_TYPE_BORDER);
    lsaB.append(" ");
    lsaB.append(this->asId);
    if(!netwksToInject.empty())
    {
        lsaB.append(" ");
        lsaB.append(MESSAGE_TYPE_INJECTED);
        lsaB.append(netwksToInject);
    }
    return lsaB;
}

string BRouter::getNetwksToInject()
{
    string netwksToInject;
    set<string> netwkSet;
    for(map<string, BGPPath>::iterator it=this->preferredPaths.begin(); 
        it!=this->preferredPaths.end(); ++it)
    {
        //dont inject netwks learnt from internal BGP peers
        if(this->iBGPpeers.find((it->second).routerId) != this->iBGPpeers.end()) 
            continue;

        BGPPath bPath = it->second;
        for(set<string>::iterator netwkIt = bPath.netwks.begin();
            netwkIt!=bPath.netwks.end(); ++netwkIt)
        {
            if(this->intradomainNetwks.find(*netwkIt) == this->intradomainNetwks.end()
                && netwkSet.find(*netwkIt) == netwkSet.end())
            {
                netwksToInject.append(" ");
                netwksToInject.append(*netwkIt);
                netwkSet.insert(*netwkIt);
            }
        }
    }
    return netwksToInject;
}

bool BRouter::verifyAndUpdateTopology(string lsa)
{
    if(LSA::hasOptions(lsa))
    {
        string asId;
        if(LSA::isFromBorderRouter(lsa, asId))
        {
            if(asId != this->asId)
            {
                if(isAttachedToMyNetwks(LSA::getSrcNetNum(lsa)))
                    updateRouterCache(eBGPpeers, lsa);
                return false;
            }
            else
                updateRouterCache(iBGPpeers, lsa);
        }
        else
        {
            if(asId != this->asId)
            {
                return false;       //Discard LSA from an interior router of different AS.
            }
        }
    }

    return Router::verifyAndUpdateTopology(lsa);
}

void BRouter::handleDeadRouters()
{
    set<string> deadiPeers = this->getDeadRouters(iBGPpeers);
    this->removeFromCachedPaths(deadiPeers);
    
    set<string> deadePeers = this->getDeadRouters(eBGPpeers);
    this->removeFromCachedPaths(deadePeers);
    
    this->withdrawRoutes(deadiPeers);
    this->withdrawRoutes(deadePeers);

    for(set<string>::iterator it=deadiPeers.begin(); it!=deadiPeers.end(); ++it)
    {
        iBGPpeers.erase(*it);
    }
    for(set<string>::iterator it=deadePeers.begin(); it!=deadePeers.end(); ++it)
    {
        eBGPpeers.erase(*it);
    }
}

void BRouter::forwardLSA(string lsa)
{
    if(!LSA::hasOptions(lsa))
        LSA::appendOptions(lsa, this->asId);
    Router::forwardLSA(lsa);
}

void BRouter::buildTempList(map<string, RouteEntry>::iterator confRE, 
    map<string, RouteEntry>& tempList, map<string, RouteEntry> confList)
{    
    for(map<string, RouterCache*>::iterator it=this->iRouters.begin();
        it!=this->iRouters.end(); ++it)
    {
        set<string> netwks = LSA::getNetwks(it->second->getLSA());
        this->removeInjectedNetworks(netwks, LSA::getInjectedNetworks(it->second->getLSA()));
        Router::buildTempList(confRE, tempList, confList, it->second->getLSA(), netwks);
    }
}

void BRouter::removeInjectedNetworks(set<string>& netwks, set<string> injectedNetwks)
{
    for(set<string>::iterator it=injectedNetwks.begin(); it!=injectedNetwks.end(); ++it)
    {
        netwks.erase(*it);
    }
}

void BRouter::writeRoutingTable(string time)
{
    Router::writeRoutingTable(time);

    //write netwks learnt through BGP to routing table
    map<string, BGPPath> bestPathToNetwk;
    for(map<string, BGPPath>::iterator it=this->preferredPaths.begin(); 
        it!=this->preferredPaths.end(); ++it)
    {
        BGPPath bPath = it->second;
        for(set<string>::iterator netwkIt = bPath.netwks.begin();
            netwkIt!=bPath.netwks.end(); ++netwkIt)
        {
            if(this->intradomainNetwks.find(*netwkIt) == this->intradomainNetwks.end())
            {
                map<string, BGPPath>::iterator bpIt = bestPathToNetwk.find(*netwkIt);
                if(bpIt == bestPathToNetwk.end() || 
                    this->isNewPathToNetwkBetter(bpIt->second, bPath))
                {
                    bestPathToNetwk[*netwkIt] = bPath;
                }
            }
        }
    }
    
    FileWriter fw(this->rtFileName);   
    for(map<string, BGPPath>::iterator it=bestPathToNetwk.begin(); 
        it!=bestPathToNetwk.end(); ++it)
    {
        //check if netwk is advertised by an eBGPpeer.
        map<string, RouterCache*>::iterator bgpEntry = eBGPpeers.find((it->second).routerId);
        if(bgpEntry != eBGPpeers.end())
        {         
            string lsa = bgpEntry->second->getLSA();
            fw.write(time + " " + it->first + " (" + LSA::getSrcNetNum(lsa) 
                + "," + LSA::getSrcHostNum(lsa) + ")");
        }
        //if not, check if it advertised by an iBGPpeer.
        bgpEntry = iBGPpeers.find((it->second).routerId);
        if(bgpEntry != iBGPpeers.end())
        {
            string lsa = bgpEntry->second->getLSA();
            set<string> netwks = LSA::getNetwks(lsa);
            set<string> injectedNetwks = LSA::getInjectedNetworks(lsa);
            this->removeInjectedNetworks(netwks, injectedNetwks);
            RouteEntry leastCostRE;
            leastCostRE.cost = 10000;
            for(set<string>::iterator netwkIt = netwks.begin();
                netwkIt!=netwks.end(); ++netwkIt)
            {
                RouteEntry re = this->routingTable[*netwkIt];
                if(re.ip.getNetNum() != IP_NA)
                {
                    if(re.cost < leastCostRE.cost)
                    {
                        leastCostRE = re;
                    }
                }
            }
            fw.write(time + " " + it->first + " " + leastCostRE.ip.toString(false));
        }
    }

    string bgpMessage;
    bgpMessage.reserve(1 + 2 + MESSAGE_TYPE_NETWKS.size() + 1 
        + (this->intradomainNetwks.size() * 3));
    bgpMessage.append(" ");
    bgpMessage.append(this->asId);
    bgpMessage.append(" "); 
    bgpMessage.append(MESSAGE_TYPE_NETWKS);
    for(set<string>::iterator it=this->intradomainNetwks.begin();
        it!=this->intradomainNetwks.end(); ++it)
    {
        bgpMessage.append(" ");
        bgpMessage.append(*it);
    }
    writeBGPmsg(eBGPpeers, bgpMessage);
}

bool BRouter::isNewPathToNetwkBetter(BGPPath existingPath, BGPPath newPath)
{
    bool retFlag = false;
    string newSrcAs = newPath.path.substr(0, 2);
    string existingSrcAs = existingPath.path.substr(0, 2);

    if(this->customers.find(existingSrcAs) != this->customers.end())
    {
        if(this->customers.find(newSrcAs) != this->customers.end())
        {
            if(newPath.path.size() < existingPath.path.size())
                retFlag = true;
        }
    }
    else
    {
        if(this->customers.find(newSrcAs) != this->customers.end())
            retFlag = true;
        else
        {            
            if(newPath.path.size() < existingPath.path.size())
                retFlag = true;
        }
    }
    return retFlag;
}

void BRouter::writeBGPmsg(map<string, RouterCache*> bgpPeers, string bgpMessagePart)
{
    FileWriter fw(this->outFileName);
    for(map<string, RouterCache*>::iterator it=bgpPeers.begin(); it!=bgpPeers.end(); ++it)
    {
        string bgpMessage;
        string lsa = it->second->getLSA();
        string peerNetNum = LSA::getSrcNetNum(lsa);
        string peerHostNum = LSA::getSrcHostNum(lsa);
        bgpMessage.reserve(7 + 1 + 7 + 1 + 2 + 1 
            + MESSAGE_TYPE_PATHADV.size() + bgpMessagePart.size());
        for(vector<IPAddress>::iterator ips=this->ipAddresses.begin();
            ips!=this->ipAddresses.end(); ++ips)
        {
            if(ips->getNetNum() == peerNetNum)
                bgpMessage.append(ips->toString(false));
        }
        bgpMessage.append(" ");
        bgpMessage.append("(" + peerNetNum + "," + peerHostNum + ")");
        bgpMessage.append(" ");
        bgpMessage.append(this->id);
        bgpMessage.append(" ");
        bgpMessage.append(MESSAGE_TYPE_PATHADV);
        bgpMessage.append(bgpMessagePart);

        string lastMessage = "";
        bool isNewMessage = true;
        for(set<string>::iterator msgIt=sentBGPmessages.begin(); msgIt!=sentBGPmessages.end(); ++msgIt)
        {
            if(msgIt->substr(0, msgIt->find(MESSAGE_TYPE_NETWKS)) 
                == bgpMessage.substr(0, bgpMessage.find(MESSAGE_TYPE_NETWKS)))
            {
                if(*msgIt != bgpMessage)
                {
                    lastMessage = *msgIt;
                    isNewMessage = true;
                }
                else
                    isNewMessage = false;
                break;
            }
        }

        if(isNewMessage)
        {
            if(!lastMessage.empty())
            {
                sentBGPmessages.erase(lastMessage);
            }
            sentBGPmessages.insert(bgpMessage);
            fw.write(bgpMessage);
        }
    }
}

void BRouter::readHBGP()
{
    FileReader fr;
    vector<string> lines = fr.read("HBGP.txt");
    for(vector<string>::iterator it=lines.begin(); it!=lines.end(); ++it)
    {
        if(it->find(this->asId) == string::npos)
            continue;
        if(it->find(DELIM_PC) != string::npos)
        {
            if(it->substr(0, 2) == this->asId)
                this->customers.insert(it->substr(2 + DELIM_PC.size(), 2));
            else
                this->providers.insert(it->substr(0, 2));
        }
        else if(it->find(DELIM_PP) != string::npos)
        {
            if(it->substr(0, 2) == this->asId)
            {
                this->peers.insert(it->substr(2 + DELIM_PP.size(), 2));
            }
            else
                this->peers.insert(it->substr(0, 2));
        }
        else if(it->find(DELIM_PP_ALT) != string::npos)
        {
            if(it->substr(0, 2) == this->asId)
            {
                this->peers.insert(it->substr(2 + DELIM_PP_ALT.size(), 2));
            }
            else
                this->peers.insert(it->substr(0, 2));
        }
    }
}

set<string> BRouter::readNetwks()
{
    set<string> bgpMessages = Router::readNetwks();
    for(set<string>::iterator it=bgpMessages.begin();
        it!=bgpMessages.end(); ++it)
    {
        if(BGP::getRouterId(*it) != this->id)
        {
            this->processBGPmessage(*it); 
        }
    }
    this->handleDeadRouters();
    return bgpMessages;
}

void BRouter::processBGPmessage(string bgpMessage)
{
    if(bgpMessage.find(MESSAGE_TYPE_WITHDRAW) != string::npos)
    {
        this->handleWithdrawMsg(bgpMessage);
        return;
    }

    BGPPath bPath;
    bPath.routerId = BGP::getRouterId(bgpMessage);
    bPath.path = BGP::getPath(bgpMessage);
    bPath.netwks = BGP::getNetwks(bgpMessage);
    
    if(setPreferredPath(bPath))
    {
        string srcAS = bPath.path.substr(0, 2);
        string routerId = BGP::getRouterId(bgpMessage);
        if(this->iBGPpeers.find(routerId) == this->iBGPpeers.end())     //do not forward messages from internal routers to internal routers
            this->exportPathToInternalPeers(bgpMessage, bPath);
        if(this->customers.find(srcAS) != this->customers.end())
        {
            this->exportPathToProviders(bgpMessage, bPath);
            this->exportPathToPeers(bgpMessage, bPath);
            this->exportPathToCustomers(bgpMessage, bPath);
        }
        else if(this->peers.find(srcAS) != this->peers.end() ||
                this->providers.find(srcAS) != this->providers.end())
            this->exportPathToCustomers(bgpMessage, bPath);
    }    
}

bool BRouter::setPreferredPath(BGPPath bPath)
{
    bool newPreferredPath = false;
    string destAS = bPath.path.substr(bPath.path.size() - 2, 2);
    map<string, BGPPath>::iterator currPrefPath = this->preferredPaths.find(destAS);
    if(currPrefPath == this->preferredPaths.end())
    {
        this->preferredPaths[destAS] = bPath;
        newPreferredPath = true;
    }
    else
    {
        if(this->isABetterPath(this->preferredPaths[destAS], bPath) 
            || this->isAnUpdateMessage(bPath))
        {
            preferredPaths[destAS] = bPath;
            newPreferredPath = true;
        }
        else
            newPreferredPath = false;
    }
    this->cachedPaths.push_back(bPath);
    return newPreferredPath;
}

bool BRouter::isABetterPath(BGPPath existingPath, BGPPath newPath)
{
    bool retFlag = false;
    string destAS = newPath.path.substr(newPath.path.size() - 2, 2);

    if(newPath.path == existingPath.path)
        return false;
    
    string newSrcAs = newPath.path.substr(0, 2);
    string existingSrcAs = existingPath.path.substr(0, 2);

    if(this->customers.find(existingSrcAs) != this->customers.end())
    {
        if(this->customers.find(newSrcAs) != this->customers.end())
        {
            if(newPath.path.size() < existingPath.path.size())
                retFlag = true;
            else if(newPath.path.size() == existingPath.path.size())     //if path lengths are equal
            {
                //least internal cost to border router
                retFlag = this->hasLeastInternalCost(existingPath, newPath);
            }
        }
    }
    else
    {
        if(this->customers.find(newSrcAs) != this->customers.end())
            retFlag = true;
        else
        {            
            if(newPath.path.size() < existingPath.path.size())
                retFlag = true;
            else if(newPath.path.size() == existingPath.path.size())     //if path lengths are equal
            {
                //least internal cost to border router
                retFlag = this->hasLeastInternalCost(existingPath, newPath);
            }
        }
    }
    return retFlag;
}

bool BRouter::isAnUpdateMessage(BGPPath newPath)
{
    string destAS = newPath.path.substr(newPath.path.size() - 2, 2);
    BGPPath existingPath = this->preferredPaths[destAS];
    if(existingPath.path == newPath.path && existingPath.routerId == newPath.routerId
        && existingPath.netwks != newPath.netwks)
        return true;
    else
        return false;
}

bool BRouter::hasLeastInternalCost(BGPPath existingPath, BGPPath newPath)
{   
    bool retFlag = false; 
    if(this->eBGPpeers.find(existingPath.routerId) == this->eBGPpeers.end())
    {
        if(this->eBGPpeers.find(newPath.path) != this->eBGPpeers.end())
            retFlag = true;
        else        //both existing and new paths are advertised by internal routers
        {
            if(this->getInternalCostToRouter(newPath.routerId)
                < this->getInternalCostToRouter(existingPath.routerId))
                retFlag = true;
        }
    }
    return retFlag;
}

int BRouter::getInternalCostToRouter(string routerId)
{
    int leastCost = 1000;
    RouterCache *entry = this->iRouters[routerId];
    string lsa = entry->getLSA();
    set<string> netwks = LSA::getNetwks(lsa);
    this->removeInjectedNetworks(netwks, LSA::getInjectedNetworks(lsa));
    for(set<string>::iterator it=netwks.begin(); it!=netwks.end(); ++it)
    {
        RouteEntry re = this->routingTable[*it];
        if(re.cost < leastCost)
            leastCost = re.cost;
    }
    return leastCost;
}

void BRouter::exportPathToInternalPeers(string bgp, BGPPath bPath)
{
    FileWriter fw(this->outFileName);
    for(map<string, RouterCache*>::iterator it=this->iBGPpeers.begin(); it!=this->iBGPpeers.end();
        ++it)
    {
        string lsa = it->second->getLSA();
        //string srcIP = this->ipAddresses.front().toString(false);
        string srcIP;
        for(vector<IPAddress>::iterator ips=this->ipAddresses.begin();
            ips!=this->ipAddresses.end(); ++ips)
        {
            bool found = true;
            for(map<string, RouterCache*>::iterator ebgp=this->eBGPpeers.begin(); ebgp!=this->eBGPpeers.end();
                ++ebgp)
            {
                string peerNetNum = LSA::getSrcNetNum(ebgp->second->getLSA());
                if(ips->getNetNum() == peerNetNum)
                {
                    found = false;
                }
            }
            if(found)
            {
                srcIP = ips->toString(false);
                break;
            }
        }
        string destIP = "(" + LSA::getSrcNetNum(lsa) + "," + LSA::getSrcHostNum(lsa) + ")";
        string newbgp;
        newbgp.reserve(bgp.size());
        newbgp.append(srcIP + " " + destIP + " " + this->id + " ");
        int offset = bgp.find(MESSAGE_TYPE_PATHADV);
        newbgp.append(bgp.substr(offset, bgp.size() - offset));

        string destAS = (bPath.path).substr((bPath.path).size() - 2, 2);
        pair<string, string> pr = make_pair(it->first, destAS);
        this->exportedPaths[bPath.routerId].insert(pr);
        fw.write(newbgp);
    }
}

void BRouter::exportPathToProviders(string bgp, BGPPath bPath)
{
    this->exportPathToOtherDomains(bgp, this->providers, bPath);
}

void BRouter::exportPathToPeers(string bgp, BGPPath bPath)
{
    this->exportPathToOtherDomains(bgp, this->peers, bPath);
}

void BRouter::exportPathToCustomers(string bgp, BGPPath bPath)
{
    this->exportPathToOtherDomains(bgp, this->customers, bPath);
}

void BRouter::exportPathToOtherDomains(string bgp, set<string> relation, BGPPath bPath)
{
    FileWriter fw(this->outFileName);
    for(map<string, RouterCache*>::iterator it=this->eBGPpeers.begin(); it!=this->eBGPpeers.end();
        ++it)
    {
        string routerId = it->first;
        string lsa = it->second->getLSA();
        string peerNetNum = LSA::getSrcNetNum(lsa);
        string peerAsId = LSA::getAsId(lsa);
        if(relation.find(peerAsId) == relation.end() ||
            routerId == BGP::getRouterId(bgp))
            continue; 

        string srcIP;
        for(vector<IPAddress>::iterator ips=this->ipAddresses.begin();
            ips!=this->ipAddresses.end(); ++ips)
        {
            if(ips->getNetNum() == peerNetNum)
            {
                srcIP = ips->toString(false);
                break;
            }
        }
        string destIP = "(" + LSA::getSrcNetNum(lsa) + "," + LSA::getSrcHostNum(lsa) + ")";
        string newbgp;
        newbgp.reserve(bgp.size() + 3);
        newbgp.append(srcIP + " " + destIP + " " + this->id + " ");
        int offset = bgp.find(MESSAGE_TYPE_PATHADV);
        newbgp.append(bgp.substr(offset, bgp.size() - offset));
        newbgp.insert(offset + MESSAGE_TYPE_PATHADV.size() + 1, this->asId + " ");
        
        string destAS = (bPath.path).substr((bPath.path).size() - 2, 2);
        pair<string, string> pr = make_pair(it->first, destAS);
        this->exportedPaths[bPath.routerId].insert(pr);

        fw.write(newbgp);
    }
}

void BRouter::removeFromCachedPaths(set<string> deadPeers)
{
    for(set<string>::iterator deadPeerIt=deadPeers.begin(); deadPeerIt!=deadPeers.end(); ++deadPeerIt)
    {
        //update Cached Paths
        for(vector<BGPPath>::iterator it=this->cachedPaths.begin();
            it!=this->cachedPaths.end();)
        {
            if(it->routerId == *deadPeerIt)
            {
                this->cachedPaths.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
}

void BRouter::withdrawRoutes(set<string> deadPeers)
{
    for(set<string>::iterator deadPeerIt=deadPeers.begin(); deadPeerIt!=deadPeers.end(); ++deadPeerIt)
    {
        //update Preferred paths
        set<string> pathsToErase;
        for(map<string, BGPPath>::iterator it=this->preferredPaths.begin();
            it!=this->preferredPaths.end(); ++it)
        {
            if((it->second).routerId == *deadPeerIt)
            {
                BGPPath bPath = this->getAltPreferredPath(it->first);
                if(!bPath.routerId.empty())
                {
                    if((it->second).path != bPath.path)
                    {
                        this->sendWithdrawMessage(it->first, *deadPeerIt);
                        this->sendAltBGPPath(*deadPeerIt, bPath);
                    }
                    this->preferredPaths[it->first] = bPath;
                }
                else
                {
                    this->sendWithdrawMessage(it->first, *deadPeerIt);
                    pathsToErase.insert(it->first);
                }
            }
        }
        for(set<string>::iterator it=pathsToErase.begin(); it!=pathsToErase.end(); ++it)
        {
            this->preferredPaths.erase(*it);
        }
        this->exportedPaths.erase(*deadPeerIt);
    }
}

BGPPath BRouter::getAltPreferredPath(string destAS)
{
    BGPPath preferredPath;
    preferredPath.routerId = "";
    for(vector<BGPPath>::iterator it=this->cachedPaths.begin();
        it!=this->cachedPaths.end(); ++it)
    {
        string currDestAS = (it->path).substr((it->path).size() - 2, 2);
        if(currDestAS == destAS)
        {
            if(preferredPath.routerId.empty() || this->isABetterPath(preferredPath, *it))
                preferredPath = *it;
        }
    }

    return preferredPath;
}

void BRouter::sendWithdrawMessage(string destAS, string deadPeer)
{
    FileWriter fw(this->outFileName);
    set<pair<string, string> > expBPaths = this->exportedPaths[deadPeer];
    for(set<pair<string, string> >::iterator it=expBPaths.begin(); it!=expBPaths.end(); ++it)
    {
        pair<string, string> pr = *it;
        if(pr.second != destAS)
            continue;
        bool isExtBGP = true;
        map<string, RouterCache*>::iterator bgpPeer = eBGPpeers.find(pr.first);
        if(bgpPeer == eBGPpeers.end())
        {
            isExtBGP = false;
            bgpPeer = iBGPpeers.find(pr.first);
            if(bgpPeer == iBGPpeers.end())
                continue;
        }
        
        string lsa = bgpPeer->second->getLSA();
        
        string srcIP = this->getSrcIPforBGP(lsa, isExtBGP);
        string destIP = "(" + LSA::getSrcNetNum(lsa) + "," + LSA::getSrcHostNum(lsa) + ")";
        fw.write(srcIP + " " + destIP + " " + this->id + " " 
            + MESSAGE_TYPE_WITHDRAW + " " + destAS);
    }
}

void BRouter::sendAltBGPPath(string deadPeer, BGPPath bPath)
{
    FileWriter fw(this->outFileName);
    string destAS = (bPath.path).substr((bPath.path).size() - 2, 2);
    set<pair<string, string> > expBPaths = this->exportedPaths[deadPeer];
    for(set<pair<string, string> >::iterator it=expBPaths.begin(); it!=expBPaths.end(); ++it)
    {
        pair<string, string> pr = *it;
        if(pr.second != destAS)
            continue;
        bool isExtBGP = true;
        map<string, RouterCache*>::iterator bgpPeer = eBGPpeers.find(pr.first);
        if(bgpPeer == eBGPpeers.end())
        {
            isExtBGP = false;
            bgpPeer = iBGPpeers.find(pr.first);
            if(bgpPeer == iBGPpeers.end())
                continue;
        }
        
        string lsa = bgpPeer->second->getLSA();
        
        string srcIP = this->getSrcIPforBGP(lsa, isExtBGP);
        string destIP = "(" + LSA::getSrcNetNum(lsa) + "," + LSA::getSrcHostNum(lsa) + ")";
        string newbgp;
        newbgp.append(srcIP + " " + destIP + " " + this->id + " ");
        newbgp.append(MESSAGE_TYPE_PATHADV + " ");
        if(isExtBGP)
            newbgp.append(this->asId + " ");
        newbgp.append(bPath.path + " ");
        newbgp.append(MESSAGE_TYPE_NETWKS);
        for(set<string>::iterator netIt=bPath.netwks.begin();
            netIt!=bPath.netwks.end(); ++netIt)
        {
            newbgp.append(" ");
            newbgp.append(*netIt);
        }
        
        this->exportedPaths[bPath.routerId].insert(make_pair(bgpPeer->first, destAS));

        fw.write(newbgp);
    }
}

string BRouter::getSrcIPforBGP(string lsa, bool isExtBGP)
{
    string srcIP;
    if(isExtBGP)
    {
        string peerNetNum = LSA::getSrcNetNum(lsa);
        for(vector<IPAddress>::iterator ips=this->ipAddresses.begin();
            ips!=this->ipAddresses.end(); ++ips)
        {
            if(ips->getNetNum() == peerNetNum)
            {
                srcIP = ips->toString(false);
                break;
            }
        }
    }
    else
    {
        for(vector<IPAddress>::iterator ips=this->ipAddresses.begin();
            ips!=this->ipAddresses.end(); ++ips)
        {
            bool found = true;
            for(map<string, RouterCache*>::iterator ebgp=this->eBGPpeers.begin(); ebgp!=this->eBGPpeers.end();
                    ++ebgp)
            {
                string peerNetNum = LSA::getSrcNetNum(ebgp->second->getLSA());
                if(ips->getNetNum() == peerNetNum)
                {
                    found = false;
                }
            }
            if(found)
            {
                srcIP = ips->toString(false);
                break;
            }
        }
    }
    return srcIP;
}

void BRouter::handleWithdrawMsg(string bgp)
{
    string destAS = bgp.substr(bgp.size() - 2, 2);
    string routerId = BGP::getRouterId(bgp);
    //update Cached Paths
    for(vector<BGPPath>::iterator it=this->cachedPaths.begin();
        it!=this->cachedPaths.end();)
    {
        string currDestAS = (it->path).substr((it->path).size() - 2, 2);
        if(currDestAS == destAS
            && it->routerId == routerId)
        {
            this->cachedPaths.erase(it);
        }
        else
        {
            ++it;
        }
    }

    if(this->preferredPaths.find(destAS) == this->preferredPaths.end())
        return;

    BGPPath bPath = this->getAltPreferredPath(destAS);
    if(!bPath.routerId.empty())
    {
        if(this->preferredPaths[destAS].path != bPath.path)
        {
            this->sendWithdrawMessage(destAS, routerId);
            this->sendAltBGPPath(routerId, bPath);
        }
        this->preferredPaths[destAS] = bPath;
    }
    else
    {
        this->sendWithdrawMessage(destAS, routerId);
        this->preferredPaths.erase(destAS);
    }
}
