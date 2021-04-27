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


enum CMD {           // types of commands in HTTP request
    MSG,        // someone is sending you a message
    MSG_OUT,    // you are sending a message to someone
    GRP_CREATE, // creating a new group/list
    GRP_DELETE, // deleting a group
    GRP_ADD,    // adding a user to a group/list
    GRP_RM      // removing a user from a group/list
};


struct State {
    std::vector<Service> services;
    bool sendInfo(Sock& client, char* request);
    bool processCommand(Sock& client, char* request);    
};


#endif // INCLUDED_STATE_H