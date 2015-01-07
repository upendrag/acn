#ifndef CONSTANTS_H
#define CONSTANTS_H

#include<string>

static const int LSA_MIN_LENGTH = 32;
static const int IP_STR_LEN = 7;
static const int MAX_HOLDING_TIME = 30;

static const std::string MESSAGE_TYPE_LSA = "LSA";
static const std::string MESSAGE_TYPE_NETWKS = "NETWKS";
static const std::string BCAST_IP= "99";     
static const std::string IP_NA = "--";

static const std::string FILES_DIR = "files/";
static const std::string FILE_EXT_TXT = ".txt";
static const std::string FILE_PREFIX_OUT = "out";
static const std::string FILE_PREFIX_NET = "NET";
static const std::string FILE_PREFIX_RT = "RT";

/*For border routers*/
static const std::string MESSAGE_TYPE_OPTIONS = "OPTIONS";
static const std::string MESSAGE_TYPE_BORDER = "BORDER";
static const std::string MESSAGE_TYPE_INJECTED = "INJECTED";
static const std::string MESSAGE_TYPE_PATHADV = "PATHADV";
static const std::string MESSAGE_TYPE_WITHDRAW = "WITHDRAW";

static const std::string DELIM_PC = " --> ";
static const std::string DELIM_PP = " - ";
static const std::string DELIM_PP_ALT = " – ";


#endif
