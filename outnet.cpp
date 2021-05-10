#include "outnet.h"
#include "sock.h"
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
using namespace std;


OutNet::OutNet(uint32_t outNetIP, uint16_t outNetPort): service(), filters() {
    service.host = outNetIP;
    service.port = outNetPort;
    filters.push_back("RPROT_EQ_outnetmsg"); // Looking for peers only,  not all services.
}


bool OutNet::registerService(uint16_t port){ // TODO: move to client.cpp ???
    stringstream ss;
    ss << "GET / HTTP/1.1\r\n";
    ss << "Register: ";
    ss << "http:tcp:outnetmsg:127.0.0.1:" << port << ":/index.html"; // your service description
    ss << "\r\n\r\n";

    Sock sock;
    if( sock.connect(service.host, service.port) ){
        return false;
    }

    int len = ss.str().length();
    if( len != sock.write( ss.str().c_str(), len ) ){
        return false;
    }

    char buff[128];
    if( sock.readLine(buff, sizeof(buff) ) <= 0 ){
        return false;
    }
    if( nullptr == strstr(buff, "200") ){
        return false;
    }
    return true;
}


bool OutNet::query(vector<HostInfo>& peers, vector<string>& local, int ageMinutes){
    service.services.clear();
    string filt = "AGE_LT" + to_string(ageMinutes);
    filters.push_back( filt );

    bool ok = queryOutNet(select, service, peers, 0, 10, &filters);
    filters.pop_back(); // it can change the next time around
    if( !ok ) { return false; }

    // TODO: this is a hack, put local services into "local" right away.
    std::copy( begin(service.services), end(service.services), back_inserter(local) ); // TODO: move ???

// TODO: remove DEBUGGING:
    cout << Sock::ipToString(service.host) << ":" << service.port << endl;
    for(string& s: service.services){
        cout << "\t" << s << endl;
    }

    for(HostInfo& hi: peers){
        cout << Sock::ipToString(hi.host) << ":" << hi.port << endl;
        for(string& s: hi.services){
            cout << "\t" << s << endl;
        }
    }
    return true;
}
