#ifndef INCLUDED_OUTNET_H
#define INCLUDED_OUTNET_H
#include "client.h" // in lib dir
#include <vector>
#include <string>
class Service;


class OutNet{
    HostInfo service;
    std::vector<std::string> filters;
    uint32_t sel = SELECT::LSVC |SELECT::IP | SELECT::PORT | SELECT::AGE | SELECT::RKEY | SELECT::ISCHK | SELECT::RSVCF;
public:
    OutNet(uint32_t outNetIP, uint16_t outNetPort);
    bool registerService(uint16_t port);
    bool query(std::vector<Service>& peers, std::vector<std::string>& local, int ageSeconds);
};


#endif // INCLUDED_OUTNET_H