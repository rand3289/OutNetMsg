#include "config.h"
#include "sock.h"   // lib dir
#include "client.h" // lib dir
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
using namespace std;
#include <chrono>
using namespace std::chrono;


struct Service {
    uint32_t ip;
    uint16_t port;
    system_clock::time_point lastSeen;
    unsigned char key[32];
    bool keyVerified;
    // Contact???
};


class OutNet{
    HostInfo service;
    vector<string> filters;
    uint32_t sel = SELECTION::IP | SELECTION::PORT | SELECTION::AGE | SELECTION::RKEY | SELECTION::ISCHK | SELECTION::RSVCF;
public:
    OutNet(uint32_t outNetIP, uint16_t outNetPort);
    bool registerService();
    bool query(vector<Service>& peers);
};


OutNet::OutNet(uint32_t outNetIP, uint16_t outNetPort): service(outNetIP, outNetPort) {
    filters.push_back("RSVC_EQ_outnetmsg"); // 
    filters.push_back("AGE_LT_600"); // get last n minutes only TODO: user config.refreshTime here
}


bool OutNet::registerService(){
    string servInfo = "web:tcp:http:127.0.0.1:80:/index.html"; // your service description
    stringstream ss;
    ss << "GET / HTTP/1.1\r\n";
    ss << "Register: " << servInfo << "\r\n\r\n";

    Sock sock;
    sock.connect(service.host, service.port);
    sock.write( ss.str().c_str(), ss.str().length() );
    return true; // TODO:
}


bool OutNet::query(vector<Service>& services){
    vector<HostInfo> newData; // results will be returned here
    queryOutNet(sel, service, newData, 0, 10, &filters);

    for(HostInfo& hi: newData){
        cout << Sock::ipToString(hi.host) << ":" << hi.port << endl;
        for(string& s: hi.services){
            cout << "\t" << s << endl;
        }
    }
    return true; // TODO:
}


// TODO: create "LITE" client which does not perform signature verification
// TODO: allow clients to sign the service registration request

int main(int argc, char* argv[]){
    Config config;
    config.load(); // TODO: failed?

    Sock server;
    server.listen(config.port);
    uint16_t port = server.getPort();
    if(port != config.port){
        config.port = port;
        config.save();
    }

    OutNet outnet(config.outIP, config.outPort);
    outnet.registerService();

    vector<Service> services;
    while(true){
        if(select()){
            // accept connection and process one of 2 types of requests:
            // POST when a new message arrives or GET when GUI requests an update
            // POST uploads binary data which gets processed and put into a JSON message list
            // some posts are sent as notification to "tray app"
            // GET can get a file from /data/ dir or JSON message list
            // TODO: need messages for creating lists, moving contacts to friend list, marking msgs "read" etc.
        }
        outnet.query(services); //  pull updates from outnet TODO: if t > blah
    }
}