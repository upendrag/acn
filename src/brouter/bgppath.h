#ifndef BGPPATH_H
#define BGPPATH_H

#include<string>
#include<set>

class BGPPath
{
    public:
        std::string routerId;
        std::string path;
        std::set<std::string> netwks;
        bool operator==(const BGPPath&);

        BGPPath();
        BGPPath(const BGPPath&);
};

#endif
