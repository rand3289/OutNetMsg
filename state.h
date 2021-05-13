#ifndef INCLUDED_STATE_H
#define INCLUDED_STATE_H
#include "sock.h"
#include "client.h"
#include <vector>
#include <map>
#include <set>
#include <string>
#include <cstring> // memcmp()
#include <chrono>


struct Key { // TODO: merge Key and PubKey
    static constexpr int KEY_SIZE = 32;
    unsigned char key[KEY_SIZE];
    bool operator<(const Key& rhs) const { return memcmp( rhs.key, key, sizeof(key)) > 0; }
    bool less(const Key& rhs) const { return memcmp( rhs.key, key, sizeof(key)) > 0; }
    bool operator==(const Key& rhs) const { return 0==memcmp(key, rhs.key, sizeof(key) ); }
    Key& operator=(const Key& rhs) { memcpy(key, rhs.key, sizeof(key)); return *this; }
    std::string toString() const;
    bool fromString(const char* str); // scan key from a hex "string"
    bool fromString(const std::string& str){ return fromString( str.c_str() ); }
};


struct Service {
    uint32_t ip;
    uint16_t port;
    Key key;
//    std::chrono::system_clock::time_point lastSeen;
//    bool keyVerified;
    std::string name; // name set by key owner during INVITE?  // TODO: comment (set by you)
};


class State {
    std::string pass;                                    // friendWord is a password to become your friend
    std::set<std::string> services;                      // local services - find OutNetTray here
    std::vector<std::string> newMessages;                // new incoming messages to be sent to the client
    std::map<Key, std::vector<std::string>> messages;    // incoming messages already sent to the client
    std::map<Key, std::vector<std::string>> outMessages; // unsent messages (recepient is offline)
    std::map<Key, Service> peers;                        // known OutNetMsg peers received from OutNet service
    std::map<std::string, std::set<Key>> groups;      // all user groups

    bool msgFrom(const std::string& key, const std::string& signature, const std::vector<std::string>& msgs);
    bool msgTo(const std::string& key, const std::string& msg, const std::string& group); // send msg to 1 user
    bool msgTo(const Key& key, const std::string& msg, const std::string& group); // send a message to 1 user
    bool msgGrp(const std::string& group, const std::string& msg); // send a message to a group
    bool sendMessages(); // send all messages from outMessages "queue"
    bool saveMessages(); // all newMessages are appended to the saved messages file
    bool saveGroup(std::string& name);   // when groups are created/deleted/updated they need to be saved to disk
    bool deleteGroup(std::string& name); // delete a single group
public:
    State();
    bool addPeers(std::vector<HostInfo>& newPeers);
    bool addServices(std::vector<std::string>& newServices);
    bool sendInfo(Sock& client, char* request);
    bool processCommand(Sock& client, char* request);
    bool loadMessages();
    bool loadGroups();
};


#endif // INCLUDED_STATE_H