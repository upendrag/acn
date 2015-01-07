#ifndef ROUTERCACHE_H
#define ROUTERCACHE_H

#include<string>

class RouterCache
{
    private:
        int bcastId;
        int holdingTime;
        std::string lsa;

    public:
        RouterCache();
        ~RouterCache();
        int getBcastId();
        void setBcastId(int);
        int getHoldingTime();
        void resetHoldingTime();
        void decrementHoldingTime();
        std::string getLSA();
        void setLSA(std::string);
};

#endif

