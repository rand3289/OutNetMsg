#ifndef INCLUDED_STATE_H
#define INCLUDED_STATE_H
#include "sock.h"
#include <vector>
#include <map>
#include <chrono>
#include <cstring> // memcmp()


static constexpr int KEY_SIZE = 32;

struct Key {
    unsigned char key[KEY_SIZE];
    bool operator<(const Key& rhs) const { return memcmp( rhs.key, key, sizeof(key)) > 0; }
    bool less(const Key& rhs) const { return memcmp( rhs.key, key, sizeof(key)) > 0; }
    Key& operator=(const Key& rhs) { memcpy(key, rhs.key, sizeof(key)); return *this; }
//    Key& operator=(const Key&& rhs) { memcpy(key, rhs.key, sizeof(key)); return *this; }
    bool fromString(const char* str); // scan key from a hex "string"
    bool fromString(const std::string& str){ return fromString( str.c_str() ); }
};


/*
struct Message {
    std::string time;
    std::string msg;
    Key from;
    bool read;
//    unsigned char signature[64];
//    bool incoming; // or outgoing???  // TODO: is this needed???
};
*/
/*
struct Contact{
    std::string key;     // public key of a user
    std::string name;    // just like a key, nick is set by the user of the key
    std::string comment; // comments are set by you and are never shared with other users
    int level;      // 0 are your best friends (notify any time)
};
*/


struct Service {
    uint32_t ip;
    uint16_t port;
    std::chrono::system_clock::time_point lastSeen;
    Key key;
    bool keyVerified;
    // TODO: add name and comment (name set by key owner [during INVITE???]. comment set by you)
};


class State {
    std::string pass;                                    // friendWord is a password to become your friend
    std::vector<std::string> services;                   // local services - find OutNetTray here
    std::vector<std::string> newMessages;                // new incoming messages to be sent to the client
    std::map<Key, std::vector<std::string>> messages;    // incoming messages already sent to the client
    std::map<Key, std::vector<std::string>> outMessages; // unsent messages (recepient is offline)
    std::map<Key, Service> peers;                        // known OutNetMsg peers received from OutNet service
    std::map<std::string, std::vector<Key>> groups;      // all user groups

    bool msgFrom(const std::string& key, const std::string& time, const std::string& msg, const std::string& signature);
    bool msgTo(const std::string& key, const std::string& msg);  // send a message to a single user
    bool msgGrp(const std::string& key, const std::string& msg); // send a message to a group
    bool saveMessages(); // all newMessages are appended to the saved messages file
    bool saveGroups();   // when groups are created/deleted/updated they need to be saved to disk
public:
    State(std::string& friendWord);
    bool addPeers(std::vector<Service>& newPeers);
    bool addServices(std::vector<std::string>& newServices);
    bool sendInfo(Sock& client, char* request);
    bool processCommand(Sock& client, char* request);
    bool loadMessages();
    bool loadGroups();
};


#endif // INCLUDED_STATE_H