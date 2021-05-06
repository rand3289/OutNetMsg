#ifndef INCLUDED_CONFIG_H
#define INCLUDED_CONFIG_H
#include <stdint.h> // uint32_t


struct Config {           // configuration file (settings)
    uint32_t outIP;       // OutNet IP
    uint16_t outPort;     // OutNet port number
    uint16_t serverPort;  // server port number
    int refreshTime;      // OutNet update interval seconds
    int readWriteTimeout; // server send() recv() timeout in seconds

    bool load();
    bool save();
};


#endif // INCLUDED_CONFIG_H
