#ifndef INCLUDED_CONFIG_H
#define INCLUDED_CONFIG_H
#include <string>

// without knowing the "friend word" friend requests will be denied
struct Config {
    std::string friendWord; // friendword
    int refreshTime;        // OutNet update interval seconds
    int readWriteTimeout;   // server send() recv() timeout in seconds
    uint32_t outIP;         // OutNet IP
    uint16_t outPort;       // OutNet port number
    uint16_t serverPort;    // server port number
    uint16_t trayPort;      // tray icon notification port number

    bool load();
    bool save();
};


struct Contact{
    std::string key;     // public key of a user
    std::string name;    // just like a key, nick is set by the user of the key
    std::string comment; // comments are set by you and are never shared with other users
    int level;      // 0 are your best friends (notify any time)
};


// block list
// friend list
// invite list - invitation to be a friend (knows your friend word)
// custom message groups of friends - share groups TODO: implement later

#endif // INCLUDED_CONFIG_H