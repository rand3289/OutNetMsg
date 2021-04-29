#include "state.h"
#include "sock.h"
#include <cstring> // memcmp()
#include <string>
#include <iostream>
#include "json.hpp"       // lib dir
using namespace nlohmann; // json.hpp
using namespace std;
void writeStatus(Sock& conn, int status, const string& reason); // in webs.cpp


enum INFO {   // types of info requested by GUI - used by sendInfo()
    msgNew=0, // get ALL new messages
    msgUser,  // get ALL messages for a user
    grpList,  // get a list of groups
    grpUsers, // get a list of users in a group
};


bool State::sendInfo(Sock& client, char* request){
    int requestType = strtol(request, 0, 10); // 10=decimal.  request format: /info?2
    switch(requestType){
        case INFO::msgNew: // send newMessages and move them to messages
            // TODO: send
            saveMessages();
            break;
        case INFO::msgUser:
            // TODO: scan user key from request and get ALL messages for that key
            // sort by time and send newest first
            break;
        case INFO::grpList: // TODO: store this info in *.json files and just send files ???
            // TODO:
            break;
        case INFO::grpUsers:
            // TODO:
            break;
        default: 
            cerr << "WARNING: unknown request type from client: " << requestType << endl;
            break;
    }
    return true;
}


// receive a message from a user
bool State::msgFrom(const string& key, const string& time, const string& msg, const string& signature){
    // TODO: check key against blacklist??? or should filtering be done on IP level?
    // TODO: verify signature
    // TODO: timestamp as int?
    unsigned char binKey[KEY_SIZE];
//    newMessages.emplace_back(time, msg, binKey, false);
    // TODO: send message to OutNetTray
    return true;
}


// send message to a user
bool State::msgTo(const string& key, const string& msg){
    unsigned char binkey[KEY_SIZE]; // TODO: convert key to binary
    for(Service& peer: peers){
        if( 0==memcmp(peer.key, binkey, sizeof(binkey) ) ){
            json msg;
            msg["from"] = ""; // TODO: get my key
            msg["time"] = ""; // TODO: add timestamp
            msg["msg"] = msg;
            msg["signature"] = ""; // TODO: sign msg
            string m = msg.dump();
            Sock conn;
            conn.connect(peer.ip, peer.port);
            conn.write( m.c_str(), m.length() );
            // TODO: if connection/write fails, put it in a retransmitt queue
        }
    }
    return true;
}


// send message to a group
bool State::msgGrp(const string& key, const string& msg){
    return true; // TODO:
}


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


// commands can be of different types  all of them are encoded in JSON
bool State::processCommand(Sock& client, char* request){
    const char* buff = ""; // TODO: find json in request
    auto cmd = json::parse(buff);
    int type = cmd["type"];

    // TODO: implement authentication.  For now, this is our security model :) LOL
    uint32_t ip = client.getIP();
    if(type != CMD::INVITE && type!= CMD::MSG && Sock::isRoutable(ip) ){
        cout << "Denying request of type " << type << " from " << Sock::ipToString(ip) << endl;
        writeStatus(client, 403, "DENIED");
    }

    switch(type){
        case CMD::INVITE:     // invitation to become a friend
//            invite(cmd["key"], cmd["msg"]); // msg is a friend word
            break;
        case CMD::MSG:        // someone is sending you a message
            msgFrom(cmd["key"], cmd["time"], cmd["msg"], cmd["signature"]);
            break;
        case CMD::MSG_USER:    // you are sending a message to someone
            msgTo(cmd["key"], cmd["msg"]);
            break;
        case CMD::MSG_GROUP:
            msgGrp(cmd["group"], cmd["msg"]);
            break;
        case CMD::GRP_CREATE: // creating a new group/list
            break;
        case CMD::GRP_DELETE: // deleting a group
            break;
        case CMD::GRP_ADD:    // adding a user to a group/list
            break;
        case CMD::GRP_RM:     // removing a user from a group/list
            break;
    }
    return true;
}


bool State::saveMessages(){ // append newMessages to saved messages file
// TODO: store messages per user (per key) and create one file per key (filename key.msg)
// group messages need to contain the user and filename is (key-group.msg)
    std::copy(make_move_iterator(begin(newMessages)), make_move_iterator(end(newMessages)), back_inserter(messages));
    newMessages.clear();
    return true; // TODO:
}


bool State::loadMessages(){ // load into messages
    return true; // TODO:
}


bool State::saveGroups(){ // when groups are created/deleted/updated they need to be saved to disk
    return true; // TODO:
}


bool State::loadGroups(){
    return true; // TODO:
}
