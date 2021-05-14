#include "state.h"
#include "sock.h"
#include <sstream>
#include <iostream>
#include <string>
#include <cstring>        // memcmp()
#include <iomanip>        // setw(), setfill()
#include "json.hpp"       // lib dir
using namespace nlohmann; // json.hpp
using namespace std;
void writeStatus(Sock& conn, int status, const string& reason); // in webs.cpp


bool Key::fromString(const char* str){
    // TODO: 
    return true;
}


string Key::toString() const {
    stringstream ss;
    for(unsigned int i = 0; i< sizeof(key); ++i){
        ss << std::setw(2) << std::setfill('0') << std::hex << (int) key[i];
    }
    return ss.str();
}


string PERM_GROUPS[] = { "BANNED", "FRIENDS", "INVITES" }; // TODO: split INVITES into invitED and invitations

// TODO: add permanent groups: BANNED, FRIENDS and INVITES (invitation to be a friend)
State::State() {
}


enum CMD {      // commands in HTTP POST request
    MESSAGE,    // incoming message
    INVITE,     // invitation to become a friend or join a group
    LEAVE,      // message sent when leaving the group

// these are GUI commands
    MSG_USER,   // send a message to a user
    MSG_GROUP,  // send a message to a group
    MSG_SEEN,   // mark message read

    GRP_LEAVE,  // group leave request
    GRP_CREATE, // create a new group/list
    GRP_DELETE, // delete a group
    GRP_ADD,    // adding a user to a group/list
    GRP_RM,     // removing a user from a group/list
};


enum INFO {   // info requested by GUI via HTTP GET - used by sendInfo()
    msgNew=0, // get ALL new messages
    msgUser,  // get ALL messages for a user
    grpList,  // get a list of groups
    grpUsers, // get a list of users in a group
    findUser, // find all keys containing a hex number
    findText, // find all messages containing text
};


bool State::sendInfo(Sock& client, char* request){
// DEBUGGING:
    string msg = "{\"key\": \"FFFF\",\"time\": \"YYMMDDhhmmss\",\"msg\": \"test msg\"}";
    newMessages.push_back(msg);

    char* sp = strchr(request, ' '); // find space as in "&blah=blah HTTP/1.1"
    if(sp){ *sp = 0; } // null terminate the string at the space before HTTP/1.1

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
        case INFO::grpList: { // TODO: store this info in *.json files and just send files ???
            // send a list of groups  as well as a list of users with their names
            data << "[";
            bool first = true;
            for(auto& g: groups) {
                if(!first) { data << ","; } else { first=false; }
                data << "\"" << g.first << "\"";
            }
            data << "]";
            ss << data.str().length() << "\r\n\r\n";
            client.write(ss.str().c_str(), ss.str().length() );
            client.write(data.str().c_str(), data.str().length() );
            break;
        }
        case INFO::grpUsers: {
            // send a list of public keys in a group
            data << "[";
            string name = request+5;// skip "&grp=";
            auto keysIt = groups.find(name);
            if(keysIt!= groups.end() ){
                bool first = true;
                for(const Key& key: keysIt->second){
                    if(!first){ data<< ","; } else { first=false; }
                    data << "\"" << key.toString() << "\""; 
                }
            }
            data << "]";
            ss << data.str().length() << "\r\n\r\n";
            client.write(ss.str().c_str(), ss.str().length() );
            client.write(data.str().c_str(), data.str().length() );
            break;
        }
        case INFO::findUser: {
// TODO: when searching for keys, search groups to find existing keys
            char* user = strstr(request, "&user=");
            if( !user ){
                writeStatus(client, 400, "Bad Request");
            }
            string hexNum = user + strlen("&user=");
cout << "DEBUG: looking for:" << hexNum << endl;
            data << "{\"keys\": [";
            for(auto& p: peers){
                string strKey = p.second.key.toString(); // TODO: very slow !!!
cout << "DEBUG: key: " << strKey << endl;
                if( string::npos != strKey.find( hexNum) ){
                    if(data.str().length() > 10){ data << ","; } // separate keys
                    data << "\"" << strKey << "\"";
                }
            }
            data << "]}"; // close JSON array

            ss << data.str().length() << "\r\n\r\n";
            client.write(ss.str().c_str(), ss.str().length() );
            client.write(data.str().c_str(), data.str().length() );
            break;
        }
        default: 
            cerr << "WARNING: unknown request type from client: " << requestType << endl;
            writeStatus(client, 400, "Bad Request");
            break;
    }
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

// TODO: get my public key.  Sign 'data'
        string myKey = "****************************************************************";
        string signature = "****************************************************************";

        stringstream ss;  // main JSON payload
        ss << "{ type: " << CMD::MESSAGE << ",key: \"" << myKey << "\", sign: \"" << signature << "\", msgs: [" << data.str() << "]}";

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


// receive a messageS from a user over HTTP POST
bool State::msgFrom(const string& key, const string& signature, const vector<string>& msgs){
// TODO: check key against blacklist besides filtering done on IP level.
// TODO: verify signature over array of msg
// "{ type: " << CMD::MSG_IN << ",key: \"" << myKey << "\", sign: \"" << signature << "\", msgs: [" << data.str() << "]}";
// msgs contain time, group, msg
    Key binKey;
    binKey.fromString(key);
    
    stringstream ss;
    for(const string& msg: msgs){ // for each message in msgs
        ss.clear();
        ss << "{key: \"" << key << "\", message: " << msg << "}";
        newMessages.push_back(ss.str());
        messages[binKey].push_back( msg );
    }
    // TODO: send last message to OutNetTray
    return true;
}


bool State::msgTo(const string& key, const string& msg, const string& group){ // send a message to a user
    Key binKey;
    binKey.fromString(key); // convert key to binary
    return msgTo(binKey, msg, group);
}


bool State::msgTo(const Key& key, const string& msg, const string& group){ // send a message to a user
    cout << "# Sending MSG: " << msg << endl; 
    cout << "# Sending  TO: " << key.toString() << endl;
    json jmsg;
    jmsg["time"] = "";  // TODO: add timestamp
    jmsg["group"] = group;
    jmsg["msg"] = msg;
    string m = jmsg.dump();
    outMessages[key].push_back(m);
    return true;
}


// send message to a group
bool State::msgGrp(const string& group, const string& msg){
    auto grp = groups.find(group);
    if( grp == groups.end() ){
        cerr << "ERROR: Group " << group << " NOT found!" << endl;
        return false;
    }
    for(const Key& key: grp->second) { // send message for every key in the group
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
        cout << "ERROR: damn that shit did not parse!" << endl;
        writeStatus(client, 400, "Bad Request");
        return false;
    }

    // TODO: implement authentication.  For now, this is our security model :) LOL
    uint32_t ip = client.getIP();
    if(type != CMD::INVITE && type!= CMD::MESSAGE && Sock::isRoutable(ip) ){
        cout << "Denying request of type " << type << " from " << Sock::ipToString(ip) << endl;
        writeStatus(client, 403, "DENIED");
    }

    cout << "POST request type: " << type << endl;
    switch(type){
        case CMD::MESSAGE:     // someone is sending you a message
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
        case CMD::INVITE:     // invitation to become a friend or a group member
// TODO: invite contains a list of keys, user and group name
            break;
        case CMD::LEAVE: { // someone wants to leave your group
            break;
        }
        case CMD::GRP_LEAVE: { // leave a group without deleting it
            break; // TODO:
        }
        case CMD::GRP_CREATE: { // creating a new group/list
            string name = cmd["grp"].get<string>();
            auto it = groups.find(name);
            if(it==groups.end()){
                groups[name];
                saveGroup(name);
            } else {
                writeStatus(client, 409,"Conflict");
                return false;
            }
            break;
        }
        case CMD::GRP_DELETE: { // deleting a group
            string name = cmd["grp"].get<string>();
            auto it = groups.find(name);
            if(it==groups.end()){
                writeStatus(client, 404, "Not Found");
                return false;
            }
            groups.erase(it);
            deleteGroup(name);
            break;
        }
        case CMD::GRP_ADD: { // adding a user to a group/list
        // TODO: check if you created the group first !!!
        // once user is added, broadcast an invite to the rest of the group
            // TODO: if key is added to blacklist, find corresponding IPs in peers and add them to IP blacklist
            string name = cmd["grp"].get<string>();
            auto it = groups.find(name);
            if(it==groups.end()){
                writeStatus(client, 404, "Group Not Found");
                return false;
            }

            string key = cmd["key"].get<string>();
            Key binKey;
            binKey.fromString(key);
            bool ok = it->second.insert(binKey).second;
            if(!ok){
                writeStatus(client, 409,"Key Exists");
                return false;
            }
            saveGroup(name);
            break;
        }
        case CMD::GRP_RM: {    // removing a user from a group/list
        // TODO: check if you created the group first !!!
        // once user is removed, broadcast an invite to the rest of the group
            string name = cmd["grp"].get<string>();
            auto it = groups.find(name);
            if(it==groups.end()){
                writeStatus(client, 404, "Group Not Found");
                return false;
            }

            string key = cmd["key"].get<string>();
            Key binKey;
            binKey.fromString(key);

            auto keyIt = it->second.find(binKey);
            if( keyIt == it->second.end()){
                writeStatus(client, 404, "Key Not Found");
                return false;
            }
            it->second.erase(keyIt);
            saveGroup(name);
            break;
        }
        default:
            cerr << "Unknown POST request type: " << type << endl;
            writeStatus(client, 400, "Bad Request");
            return false;
    }

    writeStatus(client, 200, "OK");
    return true;
}


/************************************* SAVE / LOAD **********************************************/
// store everything under ./msgdata


bool State::saveMessages(){ // append newMessages to saved messages file
// TODO: store messages per user (per key) and create one file per key (filename key.msg)
// group messages need to contain the user and filename is (key-group.msg)
    newMessages.clear();
    return true; // TODO:
}


bool State::loadMessages(){ // load into messages
    return true; // TODO:
}


bool State::saveGroup(string& name){ // when groups are created/deleted/updated they need to be saved to disk
    return true; // TODO:
}


bool State::loadGroups(){
    return true; // TODO:
}

bool State::deleteGroup(string& name){ // mark? a group "deleted"  (or move to ./deleted folder ???)
    return true; // TODO: 
}

bool State::addPeers(std::vector<HostInfo>& newPeers){
    bool foundNew = false;
    for(HostInfo& peer: newPeers){
        if(peer.key){
            Key key;
            memcpy(key.key, (*peer.key).key, sizeof(key.key) ); // TODO: remove this hack by merging Key->PubKey
            auto found = peers.find(key);
            if( found!=peers.end() ){
                found->second.ip = peer.host;   // update them just incase they've changed
                found->second.port = peer.port; // easier to update than check if they have changed
            } else {
                Service& serv = peers[key];     // insert it
                serv.ip = peer.host;
                serv.port = peer.port;
                serv.key = key;
                foundNew = true;
            }
        }
    }
    return foundNew;
}


bool State::addServices(std::vector<std::string>& newServices){
    bool inserted = false;
    for(string& s: newServices){
        inserted |= services.insert(s).second;
    }
    return inserted;
}
