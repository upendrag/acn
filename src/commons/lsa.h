#ifndef LSA_H
#define LSA_H

#include<string>
#include<set>

class LSA
{
    public:
        static std::string getSrcNetNum(std::string);
        static std::string getSrcHostNum(std::string);
        static std::string getDestNetNum(std::string);
        static std::string getDestHostNum(std::string);
        static std::string getBcastId(std::string);
        static std::string getRouterId(std::string);
        static std::set<std::string> getNetwks(std::string);
        static std::string getAsId(std::string);
        static bool hasOptions(std::string);
        static void appendOptions(std::string&, std::string);
        static bool isFromBorderRouter(std::string, std::string&);
        static std::set<std::string> getInjectedNetworks(std::string);
};

#endif
