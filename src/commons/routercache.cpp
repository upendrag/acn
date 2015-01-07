#include"routercache.h"
#include"constants.h"

using namespace std;

RouterCache::RouterCache()
{
}

RouterCache::~RouterCache()
{
}

int RouterCache::getBcastId()
{
    return this->bcastId;
}

void RouterCache::setBcastId(int bcastId)
{
    this->bcastId = bcastId;
}

int RouterCache::getHoldingTime()
{
    return this->holdingTime;
}

void RouterCache::resetHoldingTime()
{
    this->holdingTime = MAX_HOLDING_TIME;
}

void RouterCache::decrementHoldingTime()
{
    this->holdingTime--;
}

string RouterCache::getLSA()
{
    return this->lsa;
}

void RouterCache::setLSA(string lsa)
{
    this->lsa = lsa;
}

