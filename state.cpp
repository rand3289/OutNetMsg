#include "state.h"
#include "sock.h"
#include <cstring> // memcmp()
#include <string>
#include <iostream>
#include "json.hpp"       // lib dir
using namespace nlohmann; // json.hpp
using namespace std;
void writeStatus(Sock& conn, int status, const string& reason); // in webs.cpp


bool State::sendInfo(Sock& client, char* request){
    // TODO:
}


// receive a message from user
bool State::msgFrom(const string& key, const string& msg, const string& signature){

// TODO: send message to OutNetTray
}


// send message to user
bool State::msgTo(const string& key, const string& msg){
    unsigned char binkey[32]; // TODO: convert key to binary
    for(Service& peer: peers){
        if( 0==memcmp(peer.key, binkey, sizeof(binkey) ) ){
            json msg;
            msg["from"] = ""; // TODO: get my key
            msg["msg"] = msg;
            msg["signature"] = ""; // TODO: sign msg
            string m = msg.dump();
            Sock conn;
            conn.connect(peer.ip, peer.port);
            conn.write( m.c_str(), m.length() );
        }
    }
}


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
            msgFrom(cmd["key"], cmd["msg"], cmd["signature"]);
            break;
        case CMD::MSG_USER:    // you are sending a message to someone
            msgTo(cmd["key"], cmd["msg"]);
            break;
        case CMD::MSG_GROUP:
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
}
