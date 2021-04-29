#ifndef INCLUDED_STATE_H
#define INCLUDED_STATE_H
#include "sock.h"
#include <vector>
#include <map>
#include <chrono>


static constexpr int KEY_SIZE = 32;


struct Service {
    uint32_t ip;
    uint16_t port;
    std::chrono::system_clock::time_point lastSeen;
    unsigned char key[KEY_SIZE];
    bool keyVerified;
    // Contact???
};


struct Message {
    std::string time;
    std::string msg;
    unsigned char from[KEY_SIZE];
//    unsigned char signature[64];
    bool read;
    bool incoming; // or outgoing???
};


class State {
    std::vector<Message> newMessages;
    std::vector<Message> messages;
    std::vector<std::string> outMsgQueue; // unsent messages (recepient is not on line)
    std::map<std::string, std::vector<unsigned char[KEY_SIZE]>> groups; // all user groups
    bool msgFrom(const std::string& key, const std::string& time, const std::string& msg, const std::string& signature);
    bool msgTo(const std::string& key, const std::string& msg);  // send a message to a single user
    bool msgGrp(const std::string& key, const std::string& msg); // send a message to a group
    bool saveMessages(); // all newMessages are appended to the saved messages file
    bool saveGroups();   // when groups are created/deleted/updated they need to be saved to disk
// TODO: permanent groups: INVITES, BANNED and FRIENDS
public:
    std::vector<Service> peers;
    std::vector<std::string> services; // local
    bool sendInfo(Sock& client, char* request);
    bool processCommand(Sock& client, char* request);
    bool loadMessages();
    bool loadGroups();
};


#endif // INCLUDED_STATE_H