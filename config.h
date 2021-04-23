#ifndef INCLUDED_CONFIG_H
#define INCLUDED_CONFIG_H
#include <string>

// without knowing the "friend word" friend requests will be denied
struct Config{
    uint32_t outIP;    // OutNet IP
    uint16_t outPort;  // OutNet port
    int refreshTime;   // OutNet update interval seconds
    std::string friendWord; // friendword
    uint16_t port;     // server port

    bool load();
    bool save();
};


struct Contact{
    std::string key;     // public key of a user
    std::string name;    // just like a key, nick is set by the user of the key
    std::string comment; // comments are set by you and are never shared with other users
    int level;      // 0 are your best friends (notify any time)
};

#endif // INCLUDED_CONFIG_H