#ifndef INCLUDED_STATE_H
#define INCLUDED_STATE_H
#include "sock.h"
#include <vector>
#include <chrono>


struct Service {
    uint32_t ip;
    uint16_t port;
    std::chrono::system_clock::time_point lastSeen;
    unsigned char key[32];
    bool keyVerified;
    // Contact???
};


struct Message {
    std::string time;
    std::string msg;
    unsigned char from[32];
//    unsigned char signature[64];
    bool read;
};


// permanent groups: INVITES, BANNED and FRIENDS
enum CMD {      // types of commands in HTTP request
    INVITE,     // invitation to become a friend or join a group
    MSG,        // direct or group message
    GRP_LEAVE,  // group leave request

// these are GUI commands
    MSG_USER,   // send a message to a user
    MSG_GROUP,  // send a message to a group
    MSG_SEEN,   // mark message read

    GRP_CREATE, // create a new group/list
    GRP_DELETE, // delete a group
    GRP_ADD,    // adding a user to a group/list
    GRP_RM,     // removing a user from a group/list
};


class State {
    std::vector<Message> newMessages;
    std::vector<Message> messages;
    bool msgFrom(const std::string& key, const std::string& time, const std::string& msg, const std::string& signature);
    bool msgTo(const std::string& key, const std::string& msg);
public:
    std::vector<Service> peers;
    std::vector<std::string> services; // local
    bool sendInfo(Sock& client, char* request);
    bool processCommand(Sock& client, char* request);    
};


#endif // INCLUDED_STATE_H