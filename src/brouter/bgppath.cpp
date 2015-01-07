#include"bgppath.h"

BGPPath::BGPPath()
{
}

BGPPath::BGPPath(const BGPPath& obj)
{
    this->routerId = obj.routerId;
    this->path = obj.path;
    this->netwks = obj.netwks;
}

bool BGPPath::operator==(const BGPPath& b2)
{
    return (this->routerId == b2.routerId
        && this->path == b2.path
        && this->netwks == b2.netwks);
}
