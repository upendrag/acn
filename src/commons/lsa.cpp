#include<iostream>
#include"lsa.h"
#include"logger.h"
#include"constants.h"

using namespace std;

string LSA::getSrcNetNum(string lsa)
{
    return lsa.substr(1, 2);
}

string LSA::getSrcHostNum(string lsa)
{
    return lsa.substr(4, 2);
}

string LSA::getDestNetNum(string lsa)
{
    return lsa.substr(9, 2);
}

string LSA::getDestHostNum(string lsa)
{
    return lsa.substr(12, 2);
}

string LSA::getBcastId(string lsa)
{
    return lsa.substr(23, 2);
}

string LSA::getRouterId(string lsa)
{
    return lsa.substr(20, 2);
}

set<string> LSA::getNetwks(string lsa)
{
    set<string> netwks;
    int start = lsa.find(MESSAGE_TYPE_NETWKS) + MESSAGE_TYPE_NETWKS.size() + 1;
    int end = lsa.size() - 1;
    int optionsPos = lsa.find(MESSAGE_TYPE_OPTIONS);
    if(optionsPos != static_cast<int>(string::npos))
        end = optionsPos - 2;
    //if(getRouterId(lsa) == "01")
        //Logger::writeLog(lsa);
    while(start < end)
    {        
        //if(getRouterId(lsa) == "01")
            //Logger::writeLog(lsa.substr(start, 2));
        netwks.insert(lsa.substr(start, 2));
        start += 3;
    }    
    return netwks;
}

string LSA::getAsId(string lsa)
{
    string asId;
    int borderPos = lsa.find(MESSAGE_TYPE_BORDER);
    if(borderPos != static_cast<int>(string::npos))
    {
        asId = lsa.substr(borderPos + MESSAGE_TYPE_BORDER.size() + 1, 2);
    }
    else
    {
        asId = lsa.substr(lsa.find(MESSAGE_TYPE_OPTIONS) + MESSAGE_TYPE_OPTIONS.size() + 1, 2);
    }
    return asId;
}

bool LSA::hasOptions(string lsa)
{
    return (lsa.find(MESSAGE_TYPE_OPTIONS) != string::npos);
}

void LSA::appendOptions(string& lsa, string asId)
{
    lsa.append(" ");
    lsa.append(MESSAGE_TYPE_OPTIONS);
    lsa.append(" ");
    lsa.append(asId);
}

bool LSA::isFromBorderRouter(string lsa, string& asId)
{
    bool isFrmBorderRouter = false;
    int borderPos = lsa.find(MESSAGE_TYPE_BORDER);
    if(borderPos != static_cast<int>(string::npos))
    {
        isFrmBorderRouter = true;
        asId = lsa.substr(borderPos + MESSAGE_TYPE_BORDER.size() + 1, 2);
    }
    else
    {
        asId = lsa.substr(lsa.find(MESSAGE_TYPE_OPTIONS) + MESSAGE_TYPE_OPTIONS.size() + 1, 2);
    }
    return isFrmBorderRouter;
}

set<string> LSA::getInjectedNetworks(string lsa)
{
    set<string> netwks;
    int injectedPos = lsa.find(MESSAGE_TYPE_INJECTED);
    if(injectedPos != static_cast<int>(string::npos))
    {
        int start = injectedPos + MESSAGE_TYPE_INJECTED.size() + 1;
        int end = lsa.size() - 1;
        while(start < end)
        {
            netwks.insert(lsa.substr(start, 2));
            start += 3;
        }
    }
    return netwks;
}
