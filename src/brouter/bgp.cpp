#include"bgp.h"
#include<constants.h>

using namespace std;

set<string> BGP::getNetwks(string bgp)
{
    set<string> netwks;
    int start = bgp.find(MESSAGE_TYPE_NETWKS) + MESSAGE_TYPE_NETWKS.size() + 1;
    int end = bgp.size() - 1;
    while(start < end)
    {        
        netwks.insert(bgp.substr(start, 2));
        start += 3;
    }    
    return netwks;
}

string BGP::getPath(string bgp)
{
    int start = bgp.find(MESSAGE_TYPE_PATHADV) + MESSAGE_TYPE_PATHADV.size() + 1;
    int len = bgp.find(MESSAGE_TYPE_NETWKS) - start - 1;
    return bgp.substr(start, len);
}

string BGP::getRouterId(string bgp)
{
    return bgp.substr(16, 2);
}

