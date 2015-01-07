#ifndef BGP_H
#define BGP_H

#include<string>
#include<set>

class BGP
{
    public:
        static std::set<std::string> getNetwks(std::string);
        static std::string getPath(std::string);
        static std::string getRouterId(std::string);
};

#endif
