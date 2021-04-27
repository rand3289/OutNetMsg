#include "state.h"
#include "sock.h"
#include "json.hpp" // lib dir
using namespace nlohmann; // json.hpp


bool State::sendInfo(Sock& client, char* request){
    // TODO:
}


// commands can be of different types  all of them are encoded in JSON
bool State::processCommand(Sock& client, char* request){
    const char* buff = "";
    auto cmd = json::parse(buff);
    int type = cmd["type"];
    switch(type){
        case CMD::MSG: break; // someone is sending you a message
        case CMD::MSG_OUT: break; // you are sending a message to someone
        case CMD::GRP_CREATE: break; // creating a new group/list
        case CMD::GRP_DELETE: break; // deleting a group
        case CMD::GRP_ADD: break;    // adding a user to a group/list
        case CMD::GRP_RM: break;      // removing a user from a group/list
    }
    // TODO:
}
