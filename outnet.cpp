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
    filters.push_back("RPROT_EQ_outnetmsg"); // looking for peers only.  not all services RSVC == "http"
//    filters.push_back("AGE_LT_600"); // get last n minutes only TODO: user config.refreshTime here
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


// TODO: use ageSeconds to construct a filter
bool OutNet::query(vector<Service>& services, vector<string>& local, int ageSeconds){
    service.services.clear(); // TODO: this is a hack, put local services into "local" right away.
    vector<HostInfo> newData; // results will be returned here
    bool ok = queryOutNet(sel, service, newData, 0, 10, &filters);
    if( !ok ) { return false; }
    std::copy( begin(service.services), end(service.services), back_inserter(local) );

// TODO: remove DEBUGGING:
    cout << Sock::ipToString(service.host) << ":" << service.port << endl;
    for(string& s: service.services){
        cout << "\t" << s << endl;
    }

    for(HostInfo& hi: newData){
        cout << Sock::ipToString(hi.host) << ":" << hi.port << endl;
        for(string& s: hi.services){
            cout << "\t" << s << endl;
        }
    }
    return true;
}
