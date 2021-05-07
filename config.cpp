#include "config.h"
#include "sock.h"
#include "utils.h"
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
using namespace std;


const string configName = "onmsg.cfg";

// load vars from configName
bool Config::load(){
    ifstream config (configName);
    if( !config ){
        cout << configName << " not found. Setting configuration data to defaults." << endl;
    } else { // config file exists
        vector<string> lines;
        parseLines(config, lines);
        config.close();

        for(string& line: lines){
            string key, value;
            if( keyValue(line, key, value)){
                toLower(key);
                if( key == "outnetip" ){
                    outIP = Sock::stringToIP( value.c_str() );
                } else if (key == "outnetport"){
                    outPort = strtol(value.c_str(), nullptr, 10); // base 10
                } else if (key == "serverport"){
                    serverPort = strtol(value.c_str(), nullptr, 10); // base 10
                } else if (key == "refreshtime"){
                    refreshTime = strtol(value.c_str(), nullptr, 10); // base 10
                } else if (key == "readwritetimeout"){
                    readWriteTimeout = strtol(value.c_str(), nullptr, 10); // base 10
                }
            } // if keyValue()
        } // for

        if( serverPort != Sock::ANY_PORT ) {
            cout << "Configuration data loaded successfuly." << endl;
            return true;
        }else {
            cout << "Config file is corrupted.  It will be regenerated." << endl;
        }
    } // else

    cout << "Please edit " << configName << " before running again." << endl;
    outIP = Sock::stringToIP("127.0.0.1");
    outPort = Sock::ANY_PORT;
    serverPort = Sock::ANY_PORT;
    refreshTime = 600;
    readWriteTimeout = 3;
    save();
    return false;
}


// save variables to configName
bool Config::save(){
    cout << "Saving configuration data to disk." << endl;
    ofstream config(configName);

    config << "# Configuration file for OutNetMsg service https://github.com/rand3289/OutNetMsg" << endl;
    config << "# If this file is deleted or corrupted it will be regenerated with default values." << endl;
    config << endl;
    config << "# OutNetMsg depends on OutNet service.  These settings are for your local OutNet service:" << endl;
    config << "OutNetIP="   << Sock::ipToString(outIP) << "\t# IP address of your local OutNet service" <<  endl;
    config << "OutNetPort="   << outPort << "\t# port number of your local OutNet service" <<  endl;
    config << "RefreshTime="  << refreshTime << "\t# how often to refresh peer data from OutNet service (seconds)" << endl;
    config << endl;
    config << "ServerPort="  << serverPort << "\t# port number you want OutNetMsg to run on" <<  endl;
    config << "ReadWriteTimeout=" << readWriteTimeout << "\t# OutNetMsg server port read/write timeouts (seconds)" << endl;

    return config.good();
}
