#include "config.h"
#include "sock.h"
#include <string>
using namespace std;


const string configName = "onmsg.cfg";

// load vars from configName
bool Config::load(){
    outIP = Sock::stringToIP("127.0.0.1");
    outPort = 4444;
    return true; // TODO:
}


// save variables to configName
bool Config::save(){
    return true; // TODO:
}
