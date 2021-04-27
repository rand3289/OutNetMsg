#include "config.h"
#include "state.h"
#include "client.h" // in lib dir
#include "sock.h"   // in lib dir
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <thread> // sleep_for()
using namespace std;
#include <chrono>
using namespace std::chrono;
bool sendFile(Sock& conn, char* request); // in webs.cpp
// TODO: create "LITE" client which does not perform signature verification
// TODO: allow clients to sign the service registration request

// TODO: search messages for keywords

class OutNet{
    HostInfo service;
    vector<string> filters;
    uint32_t sel = SELECT::LSVC |SELECT::IP | SELECT::PORT | SELECT::AGE | SELECT::RKEY | SELECT::ISCHK | SELECT::RSVCF;
public:
    OutNet(uint32_t outNetIP, uint16_t outNetPort);
    bool registerService();
    bool query(vector<Service>& peers, vector<string>& local, int ageSeconds);
};


OutNet::OutNet(uint32_t outNetIP, uint16_t outNetPort): service(), filters() {
    service.host = outNetIP;
    service.port = outNetPort;
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


// TODO: use ageSeconds to construct a filter
bool OutNet::query(vector<Service>& services, vector<string>& local, int ageSeconds){
    service.services.clear(); // TODO: this is a hack, put local services into "local" right away.
    vector<HostInfo> newData; // results will be returned here
    queryOutNet(sel, service, newData, 0, 10, &filters);
    std::copy( begin(service.services), end(service.services), back_inserter(local) );

    for(HostInfo& hi: newData){
        cout << Sock::ipToString(hi.host) << ":" << hi.port << endl;
        for(string& s: hi.services){
            cout << "\t" << s << endl;
        }
    }
    return true; // TODO:
}


// After init/registration, the service listens for incoming connections.
// It accepts a connection and processes one of 2 types of requests:
// POST when a message/command arrives or GET when GUI requests an update
// Messages get appended to a message list for GUI to retrieve later.
// POST data is in JSON format.
// Some posts (messages from friends) are sent as notifications to "tray app".
// POST gets an OK or DENIED/BANNED response
// GET can get a file from /data/ dir or info (messages or lists) in JSON format
int main(int argc, char* argv[]){
    initNetwork(); //

    Config config;
    config.load(); // TODO: failed?

    Sock server;
    server.listen(config.serverPort);
    uint16_t port = server.getPort();
    if(port != config.serverPort){
        config.serverPort = port;
        config.save();
    }

    cout << "Running server on port " << port << endl;
    cout << "Registering server with OutNet" << endl;

    State state;
    OutNet outnet(config.outIP, config.outPort);
    outnet.registerService();

    system_clock::time_point last = system_clock::now();
    char buff[2048];
    cout << "running..." << endl;

    while(true){
        Sock client;
        if( server.accept(client) > 0 ){
            uint32_t ip = client.getIP();
            // TODO: check if ip is blacklisted   There is a second blacklist for keys (save keys only)
            client.setRWtimeout(config.readWriteTimeout); // so I can disconnect slow clients
            int rd = client.readLine(buff, sizeof(buff));
            if(rd <= 0){ continue; }// error reading data? (connection closed/timed out)
            cout << "REQUEST: " << buff << endl;
            if( 0==strncmp(buff,"GET ",4) ){         // HTTP GET query
                if(0==strncmp(buff+4,"/info?", 6)){  // request for information (JSON)
                    state.sendInfo(client, buff+10);
                } else {                             // request for a static file
                    sendFile(client, buff+4);
                }
            } else if(0==strncmp(buff, "POST ",5) ){ // HTTP POST - this is a command or a message
                state.processCommand(client, buff+5);
            }
//            this_thread::sleep_for(seconds(2));
        }
        auto now = system_clock::now();
        auto delta = now - last;
        auto sec = duration_cast<seconds>(delta).count();
        if( sec > config.refreshTime ){
            outnet.query(state.peers, state.services, sec); //  pull updates from outnet
            last = now;
        }
    } // while
    return 0;
} // main()
