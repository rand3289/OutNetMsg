// This file is part of OutNet Messenger  https://github.com/rand3289/OutNetMsg
#ifndef INCLUDED_OUTNET_H
#define INCLUDED_OUTNET_H
#include "client.h" // in lib dir
#include <vector>
#include <string>


class OutNet{
    HostInfo service;
    std::vector<std::string> filters;
    uint32_t select = SELECT::LSVC |SELECT::IP | SELECT::PORT | SELECT::AGE | SELECT::RKEY | SELECT::ISCHK | SELECT::RSVC | SELECT::RSVCF;
public:
    OutNet(uint32_t outNetIP, uint16_t outNetPort);
    bool registerService(uint16_t port);
    bool query(std::vector<HostInfo>& peers, std::vector<std::string>& local, int ageMinutes);
};


#endif // INCLUDED_OUTNET_H