#ifndef INCLUDED_CONFIG_H
#define INCLUDED_CONFIG_H
#include <string>


// without knowing the "friend word" friend requests will be denied
struct Config {
    std::string friendWord; // friendword
    int refreshTime = 600;        // OutNet update interval seconds
    int readWriteTimeout = 3;   // server send() recv() timeout in seconds
    uint32_t outIP;         // OutNet IP
    uint16_t outPort;       // OutNet port number
    uint16_t serverPort=2778;    // server port number
//    uint16_t trayPort;      // tray icon notification port number // get it from OutNet - tray should register

    bool load();
    bool save();
};


#endif // INCLUDED_CONFIG_H
