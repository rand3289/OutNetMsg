#include "config.h"
#include "sock.h"   // lib dir
#include "client.h" // lib dir
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <thread> // sleep_for()
using namespace std;
#include <chrono>
using namespace std::chrono;
// TODO: create "LITE" client which does not perform signature verification
// TODO: allow clients to sign the service registration request


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
    bool query(vector<Service>& peers, int ageSeconds);
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


bool OutNet::query(vector<Service>& services, int ageSeconds){ // TODO: use ageSeconds to construct a filter
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


bool sendFile(Sock& conn, char* request){
    char * end = findReverse(request, "HTTP/1.1");
    if(nullptr == end){ return false; } // malformed HTTP request // TODO: return 400 (bad request)?
    *end = 0; // null terminate the path
    auto file = ltrim(rtrim(request));
    string path = "./data"; // TODO: do not allow directories or sanitize path (use <filesystem> lib)
    path += file;

    ifstream f(path, ios::binary);
    if(!f){ return false; } // TODO: 404 not found
    f.seekg(0, ios::end);
    size_t fsize = f.tellg();
    f.seekg(0, std::ios::beg);
    cout << "sending file: " << path << " (" << fsize << " bytes)" << endl;

    stringstream ssh; // ss header
    ssh << "HTTP/1.1 200 OK\r\n"; // second "\r\n" separates headers from html
//    ssh << "Content-Type: text/html\r\n";
    ssh << "Content-Type: application/octet-stream\r\n";
    ssh << "Content-Length: " << fsize << "\r\n";
    ssh << "\r\n";
    char buff[1024*1024];
    while( f.read(buff, sizeof(buff)) ){
        size_t size = f.gcount();
        conn.write(buff, size);
    }
    return true;
}


struct State {
    vector<Service> services;
    bool sendInfo(Sock& client, char* request);
    bool processCommand(Sock& client, char* request);    
};


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

    State state;
    OutNet outnet(config.outIP, config.outPort);
    outnet.registerService();

    system_clock::time_point last = system_clock::now();
    char buff[2048];

    // accept connection and process one of 2 types of requests:
    // POST when a new message arrives or GET when GUI requests an update
    // POST uploads binary data which gets processed and put into a JSON message list
    // some posts are sent as notification to "tray app"
    // POST gets an OK or DENIED/BANNED response
    // GET can get a file from /data/ dir or JSON message list
    // TODO: need messages for creating lists, moving contacts to friend list, marking msgs "read" etc.
    while(true){
        Sock client;
        if( server.accept(client) > 0 ){
            uint32_t ip = client.getIP();
            // TODO: check if ip is blacklisted   There is a second blacklist for keys (save keys only)
            client.setRWtimeout(config.readWriteTimeout); // so I can disconnect slow clients
            int rd = client.readLine(buff, sizeof(buff));
            if(rd > 0){ // error reading data? (connection closed/timed out)
//                string line = buff; // save it for debugging
//                printAscii((const unsigned char*)buff, rd);
                if( 0==strncmp(buff,"GET ",4) ){         // HTTP GET query
                    if(0==strncmp(buff+4,"/info?", 6)){  // request for information (JSON)
                        state.sendInfo(client, buff+10);
                    } else {                             // request for a static file
                        sendFile(client, buff+4);
                    }
                } else if(0==strncmp(buff, "POST ",5) ){ // HTTP POST - this is a command or a message
                    state.processCommand(client, buff+5);
                }
                this_thread::sleep_for(seconds(2));
            }
        }
        auto now = system_clock::now();
        auto delta = now - last;
        auto sec = duration_cast<seconds>(delta).count();
        if( sec > config.refreshTime ){
            outnet.query(state.services, sec); //  pull updates from outnet
            last = now;
        }
    } // while
    return 0;
} // main()
