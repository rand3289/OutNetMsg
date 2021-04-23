#ifndef INCLUDED_CONFIG_H
#define INCLUDED_CONFIG_H
#include <string>

// without knowing the "friend word" friend requests will be denied
struct Config{
    uint32_t outIP;    // OutNet IP
    uint16_t outPort;  // OutNet port
    int refreshTime;   // OutNet update interval seconds
    string friendWord; // friendword
    uint16_t port;     // server port
    Config();
    bool save();
};


struct Contact{
    char level;      // 0 = your best friends (notification any time)
    string key;     // public key of a user
    string name;    // just like a key, nick is set by the user of the key
    string comment; // comments are set by you and are never shared with other users 
};

#endif INCLUDED_CONFIG_H