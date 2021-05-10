#include "config.h"
#include "state.h"
#include "outnet.h"
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
// TODO: create "LITE" OutNet client which does not perform signature verification
// TODO: allow clients to sign the service registration request
// TODO: search messages for keywords


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
    if( !config.load() ){ // can not run without OutNet IP:PORT
        return 1;
    }

    Sock server;
    server.listen(config.serverPort);
    uint16_t port = server.getPort();
    if(port != config.serverPort){
        config.serverPort = port;
        config.save();
    }

    cout << "Running server on port " << port << endl;
    cout << "Registering server with OutNet " << Sock::ipToString(config.outIP) << ":" << config.outPort << endl;

    OutNet outnet(config.outIP, config.outPort);
    if( ! outnet.registerService(port) ){
        cerr << "Error registering OutNetMsg with OutNet service.  Is it running?  Exiting." << endl;
        return 2;
    }

    State state;
    state.loadGroups();
    state.loadMessages();

    system_clock::time_point last = system_clock::now() - seconds(config.refreshTime);
    char buff[8*1024];
    cout << "running..." << endl;

    while(true){
        Sock client;
        if( server.accept(client) > 0 ){
            uint32_t ip = client.getIP();
            // TODO: check if ip is blacklisted   There is a second blacklist for keys (save keys only)
            // recreate ip blacklist by using key blacklist and getting IPs from state.peers
            client.setRWtimeout(config.readWriteTimeout); // so I can disconnect slow clients
            int rd = client.readLine(buff, sizeof(buff));
            if(rd <= 0){ continue; }// error reading data? (connection closed/timed out)
            cout << "REQUEST: " << buff << endl;
            if( 0==strncmp(buff,"GET ",4) ){         // HTTP GET query
                if(0==strncmp(buff+4,"/?info=", 7)){ // request for information (JSON)
                    state.sendInfo(client, buff+11);
                } else {                             // request for a static file
                    sendFile(client, buff+4);
                }
            } else if(0==strncmp(buff, "POST ",5) ){ // HTTP POST - this is a command or a message
                state.processCommand(client, buff+5);
            }
            this_thread::sleep_for(seconds(1)); // otherwise winblows freaks out
        }
        auto now = system_clock::now();
        auto delta = now - last;
        auto sec = duration_cast<seconds>(delta).count();
        if( sec > config.refreshTime ){
            vector<Service> peers;
            vector<string> services;
            outnet.query(peers, services, sec); //  pull updates from outnet
            state.addPeers(peers);
            state.addServices(services);
            last = now;
        }
    } // while
    return 0;
} // main()
