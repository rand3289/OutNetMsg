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


std::string Key::toString() const {
    return string(); // TODO:
}


// TODO: add permanent groups: BANNED, FRIENDS and INVITES (invitation to be a friend [knows your friend word???] )
State::State() {
}


enum CMD {      // types of commands in HTTP request
    INVITE,     // invitation to become a friend or join a group
    MSG_IN,     // direct or group message
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


enum INFO {   // types of info requested by GUI - used by sendInfo()
    msgNew=0, // get ALL new messages
    msgUser,  // get ALL messages for a user
    grpList,  // get a list of groups
    grpUsers, // get a list of users in a group
};


bool State::sendInfo(Sock& client, char* request){
// DEBUGGING:
    string msg = "{\"key\": \"FFFF\",\"time\": \"YYMMDDhhmmss\",\"msg\": \"test msg\"},";
    newMessages.push_back(msg);

    stringstream ss, data;
    ss << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: ";
    int requestType = strtol(request, 0, 10); // 10=decimal.  request format: /?info=2
//    cout << "Request type " << requestType << endl;
    switch(requestType){
        case INFO::msgNew: { // send newMessages
            data << "[";     // JSON array
            for(unsigned int i=0; i < newMessages.size(); ++i){
                if(i>0){ data << ","; } // separate messages within the array with a coma
                data << newMessages[i];
            }
            data << "]";

            ss << data.str().length() << "\r\n\r\n";
            client.write(ss.str().c_str(), ss.str().length() );     // HTTP header
            client.write(data.str().c_str(), data.str().length() ); // JSON data
            saveMessages();
            break; 
        }
        case INFO::msgUser: { // scan user key from request and get ALL messages for that key
            data << "["; // JSON array
            Key binKey;
            binKey.fromString(request+5);// skip "&key="
            auto msgs = messages.find(binKey);
            if(msgs != end(messages)){
                for(unsigned int i=0; i < msgs->second.size(); ++i){
                    if(i>0){ data << ","; }
                    data << msgs->second[i];
                }
            }
            data << "]"; // close JSON array

            ss << data.str().length() << "\r\n\r\n";
            client.write(ss.str().c_str(), ss.str().length() );
            client.write(data.str().c_str(), data.str().length() );
            break;
        }
        case INFO::grpList: // TODO: store this info in *.json files and just send files ???
            // TODO: send a list of groups  as well as a list of users with their names
            break;
        case INFO::grpUsers:
            // TODO: send a list of public keys in a group
            break;
        default: 
            cerr << "WARNING: unknown request type from client: " << requestType << endl;
            writeStatus(client, 400, "Bad Request");
            break;
    }
    return true;
}


// receive a message from a user over HTTP POST
bool State::msgFrom(const string& key, const string& signature, const vector<string>& msgs){
    // TODO: check key against blacklist besides filtering done on IP level.
    // TODO: verify signature over array of msg
    // TODO: send last message to OutNetTray
// "{ type: " << CMD::MSG_IN << ",key: \"" << myKey << "\", sign: \"" << signature << "\", msgs: [" << data.str() << "]}";

    Key binKey;
    binKey.fromString(key);
    
    stringstream ss;
    for(const string& msg: msgs){ // for each message in msgs
        ss.clear();
        ss << "{key: \"" << key << "\", message: " << msg << "}";
        newMessages.push_back(ss.str());
        messages[binKey].push_back( msg );
    }
    return true;
}


bool State::msgTo(const string& key, const string& msg, const string& group){ // send a message to a user
    Key binKey;
    binKey.fromString(key); // convert key to binary
    return msgTo(binKey, msg, group);
}


bool State::msgTo(const Key& key, const string& msg, const string& group){ // send a message to a user
    cout << "Sending MSG: " << msg << " TO: " << key.toString() << endl;
    json jmsg;
    jmsg["time"] = "";  // TODO: add timestamp
    jmsg["group"] = group;
    jmsg["msg"] = msg;
    string m = jmsg.dump();
    outMessages[key].push_back(m);
    return true;
}


bool State::sendMessages(){
    for(auto km: outMessages){            // key-messages pair
        const Key& pk = km.first;         // message receiver's key
        vector<string>& msgs = km.second; // vector of messages for that user
        if( msgs.empty() ){ continue; }   // no messages for that user // TODO: delete this public key from map

        auto inf = peers.find(pk);        // find info for that public key
        if(inf == peers.end() ){
            cout << "Peer information for key " << pk.toString() << " not found!" << endl;
            continue;
        }

        Sock conn;
        if ( !conn.connect(inf->second.ip, inf->second.port) ){
            cerr << "Error connecting to " << inf->second.ip << ":" << inf->second.port << endl;
            continue;
        }

        stringstream data; // create JSON array of messages
        for(unsigned int i=0; i < msgs.size(); ++i){
            if(i>0){ data << ","; }
            data << msgs[i];
        }

        string myKey = "****************************************************************";     // TODO: get my public key
        string signature = "****************************************************************"; // TODO: sign 'data'

        stringstream ss;  // main JSON payload
        ss << "{ type: " << CMD::MSG_IN << ",key: \"" << myKey << "\", sign: \"" << signature << "\", msgs: [" << data.str() << "]}";

        stringstream head;
        head << "POST / HTTP/1.1\r\n";
        head << "Content-Type: application/json\r\n";
        head << "Content-Length: " << ss.str().length() << "\r\n";
        head << "\r\n";

        conn.write( head.str().c_str(), head.str().length() );   // write request header
        if( conn.write( ss.str().c_str(), ss.str().length() ) ){ // write payload (JSON data)
            msgs.clear();
        }
    } // for
    return true;
}


// send message to a group
bool State::msgGrp(const string& group, const string& msg){
    auto grp = groups.find(group);
    if( grp == groups.end() ){
        cerr << "ERROR: Group " << group << " NOT found!" << endl;
        return false;
    }
    for(Key& key: grp->second) { // send message for every key in the group
        msgTo(key, msg, group);
    }
    return true;
}


// commands can be of different types (see enum CMD) all of them are encoded in JSON
bool State::processCommand(Sock& client, char* request){
    char data[8*1024];
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
    int type = 0;
    json cmd;
    try {
        cmd = json::parse(data); // buff2
        type = cmd["type"].get<int>();
    } catch (...) {
        cout << "ERROR: God damn that shit did not parse!" << endl;
        writeStatus(client, 400, "Bad Request");
        return false;
    }

    // TODO: implement authentication.  For now, this is our security model :) LOL
    uint32_t ip = client.getIP();
    if(type != CMD::INVITE && type!= CMD::MSG_IN && Sock::isRoutable(ip) ){
        cout << "Denying request of type " << type << " from " << Sock::ipToString(ip) << endl;
        writeStatus(client, 403, "DENIED");
    }

    cout << "POST request type: " << type << endl;
    switch(type){
        case CMD::INVITE:     // invitation to become a friend
//            invite(cmd["key"], cmd["msg"]); // msg is a friend word
// TODO: invite contains a list of keys, user and group name
            break;
        case CMD::MSG_IN:     // someone is sending you a message
            msgFrom( cmd["key"].get<string>(), cmd["sign"].get<string>(), cmd["msgs"].get<vector<string>>() );
            break;
        case CMD::MSG_USER:    // you are sending a message to someone
            msgTo( cmd["key"].get<string>(), cmd["msg"].get<string>(), "" ); // group is empty
            sendMessages();
            break;
        case CMD::MSG_GROUP: {
            bool ok = msgGrp( cmd["group"].get<string>(), cmd["msg"].get<string>() );
            if(!ok){
                writeStatus(client, 404, "NOT FOUND"); // group not found (can not post to that group)
                return false;
            }
            sendMessages();
            break;
        }
        case CMD::GRP_CREATE: // creating a new group/list
            break;
        case CMD::GRP_DELETE: // deleting a group
            break;
        case CMD::GRP_ADD:    // adding a user to a group/list
            // TODO: if key is added to blacklist, find corresponding IPs in peers and add them to IP blacklist
            break;
        case CMD::GRP_RM:     // removing a user from a group/list
            break;
        default:
            cerr << "Unknown POST request type: " << type << endl;
            writeStatus(client, 400, "Bad Request");
            return false;
    }

    writeStatus(client, 200, "OK");
    return true;
}


/************************************* SAVE / LOAD **********************************************/

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
