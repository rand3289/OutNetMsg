#include "state.h"
#include "sock.h"
#include <sstream>
#include <iostream>
#include <string>
#include <cstring>        // memcmp()
#include "json.hpp"       // lib dir
using namespace nlohmann; // json.hpp
using namespace std;
void writeStatus(Sock& conn, int status, const string& reason); // in webs.cpp


bool Key::fromString(const char* str){
    // TODO: 
    return true;
}


// TODO: add permanent groups: BANNED, FRIENDS and INVITES (invitation to be a friend [knows your friend word???] )
State::State(std::string& friendWord): pass(friendWord) {
}


enum INFO {   // types of info requested by GUI - used by sendInfo()
    msgNew=0, // get ALL new messages
    msgUser,  // get ALL messages for a user
    grpList,  // get a list of groups
    grpUsers, // get a list of users in a group
};


bool State::sendInfo(Sock& client, char* request){
    int requestType = strtol(request, 0, 10); // 10=decimal.  request format: /?info=2
//    cout << "Request type " << requestType << endl;
    stringstream ss;
    ss << "HTTP/1.1 200 OK" << endl << "Content-type: application/json" << endl << "Content-length: ";
    switch(requestType){
        case INFO::msgNew: { // send newMessages and move them to messages

// DEBUGGING:
    string msg = "{\"key\": \"FFFF\",\"time\": \"YYMMDDhhmmss\",\"msg\": \"test msg\"},";
    newMessages.push_back(msg);
    newMessages.push_back(msg);
    newMessages.push_back(msg);

            unsigned long len = 4; // [{}]
            for(auto& msg: newMessages){
                len+= msg.length();
            }

            ss << len << endl << endl << "[";
            client.write(ss.str().c_str(), ss.str().length() );

            for(auto& msg: newMessages){
                client.write(msg.c_str(), msg.length() );
            }
            client.write("{}]", 3); // make an empty object at the end to take care of the last comma
            saveMessages();
            break; }
        case INFO::msgUser: { // scan user key from request and get ALL messages for that key
            unsigned long len = 4; // [{}]
            Key binKey;
            binKey.fromString(request+5);// skip "&key="
            auto msgs = messages.find(binKey);
            if(msgs != end(messages)){
                for(auto& msg: msgs->second){
                    len += msg.length();
                }
            }

            ss << len << endl << endl << "[";
            client.write(ss.str().c_str(), ss.str().length() );

            if(msgs != end(messages)){
                for(auto& msg: msgs->second){
                    client.write(msg.c_str(), msg.length() );
                }
            }

            client.write("{}]", 3); // make an empty object at the end to take care of the last comma
            break; }
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
    // TODO: check key against blacklist or should filtering be done on IP level?
    // TODO: verify signature over time+msg
    // TODO: send message to OutNetTray
    Key binKey;
    binKey.fromString(key);
    stringstream ss;
    ss << "{" << "\"key:\"" << key << ",\"time:\"" << time << ",\"msg:\"" << msg << "}";
    newMessages.push_back(ss.str());
    messages[binKey].push_back( ss.str() );
    return true;
}


// send message to a user
bool State::msgTo(const string& key, const string& msg){
    Key binKey;
    binKey.fromString(key); // convert key to binary
    cout << "Sending MSG: " << msg << " TO: " << key << endl;
    for(auto& peer: peers){
        if( 0==memcmp(&peer.second.key, binKey.key, sizeof(binKey.key) ) ){
            json msg;
            msg["from"] = ""; // TODO: get my key
            msg["time"] = ""; // TODO: add timestamp
            msg["msg"] = msg;
            msg["signature"] = ""; // TODO: sign msg
            string m = msg.dump();
            Sock conn;
            conn.connect(peer.second.ip, peer.second.port);
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


// commands can be of different types (see enum CMD) all of them are encoded in JSON
bool State::processCommand(Sock& client, char* request){
    char data[8*1024];
/*    char* buff = &data[0];
    int rd = 0;
    while( (rd=client.read(buff, sizeof(data)-(buff-data) ) ) ) {
        for(int i = 0; i < rd; ++i){ cout << buff[i]; cout.flush(); }
        buff += rd;
    }

    char* buff2 = strstr(data, "\r\n");
    if( !buff2 ){
        cout << "ERROR: HTTP request did not have a header separator." << endl;
        writeStatus(client, 400, "Bad Request");
        return false;
    }
    buff2+=2; // skip \r\n
*/
    int csize = 0;
    while( client.readLine(data,sizeof(data) ) ) {
        if( 0 == strncmp(data, "Content-Length:", 15) ) {
            csize = strtol(data+16, 0, 10); // 10 = base 10
        } else if ( 0 == strlen(data) ) { break; } // empty line - end of header
    }
    if( csize != client.read(data, csize) ){
        cerr << "ERROR reading JSON" << endl;
        return false;
    }
    data[csize] = 0; // null terminate the JSON string

    cout << "JSON: " << data << endl;
    json cmd;
    try {
        cmd = json::parse(data); // buff2
    } catch (...) {
        cout << "ERROR: God damn that shit did not parse!" << endl;
        writeStatus(client, 400, "Bad Request");
        return false;
    }
    int type = cmd["type"].get<int>();

    // TODO: implement authentication.  For now, this is our security model :) LOL
    uint32_t ip = client.getIP();
    if(type != CMD::INVITE && type!= CMD::MSG && Sock::isRoutable(ip) ){
        cout << "Denying request of type " << type << " from " << Sock::ipToString(ip) << endl;
        writeStatus(client, 403, "DENIED");
    }

    cout << "POST request type: " << type << endl;
    switch(type){
        case CMD::INVITE:     // invitation to become a friend
//            invite(cmd["key"], cmd["msg"]); // msg is a friend word
            break;
        case CMD::MSG:        // someone is sending you a message
            msgFrom( cmd["key"].get<string>(), cmd["time"].get<string>(), cmd["msg"].get<string>(), cmd["signature"].get<string>() );
            break;
        case CMD::MSG_USER:    // you are sending a message to someone
            msgTo( cmd["key"].get<string>(), cmd["msg"].get<string>() );
            break;
        case CMD::MSG_GROUP:
            msgGrp( cmd["group"].get<string>(), cmd["msg"].get<string>() );
            break;
        case CMD::GRP_CREATE: // creating a new group/list
            break;
        case CMD::GRP_DELETE: // deleting a group
            break;
        case CMD::GRP_ADD:    // adding a user to a group/list
            // TODO: if key is added to blacklist, find corresponding IPs in peers and add them to IP blacklist
            break;
        case CMD::GRP_RM:     // removing a user from a group/list
            break;
    }

    writeStatus(client, 200, "OK");
    return true;
}


bool State::saveMessages(){ // append newMessages to saved messages file
// TODO: store messages per user (per key) and create one file per key (filename key.msg)
// group messages need to contain the user and filename is (key-group.msg)
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


bool State::addPeers(std::vector<Service>& newPeers){
    return true; // TODO:
}


bool State::addServices(std::vector<std::string>& newServices){
    return true; // TODO:
}
