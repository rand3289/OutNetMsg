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
