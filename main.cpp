#include <iostream>

void registerService(uint32_t outNetIP, uint16_t outNetPort, const string& servInfo){
    string servInfo = "web:tcp:http:127.0.0.1:80:/index.html"; // your service description
    stringstream ss;
    ss << "GET / HTTP/1.1\r\n";
    ss << "Register: " << servInfo << "\r\n\r\n";

    Sock sock;
    sock.connect(outNetIP, outNetPort);
    sock.write( ss.str().c_str(), ss.str().length() );
}


void queryOutNet(HostInfo& service){
    vector<HostInfo> newData; // results will be returned here
    vector<string> filters;   //
    filters.push_back("RSVC_EQ_outnetmsg"); // 
    filters.push_back("AGE_LT_600"); // get last n minutes only TODO: user config.refreshTime here
    int32_t sel = SELECTION::IP | SELECTION::PORT | SELECTION::AGE | SELECTION::KEY | SELECTION::RSVCF;
    queryOutNet(sel, service, newData, 0, 10, &filters);

    for(HostInfo& hi: newData){
        cout << Sock::ipToString(hi.host) << ":" << hi.port << endl;
        for(string& s: hi.services){
            cout << "\t" << s << endl;
        }
    }
}


// TODO: create "LITE" client which does not perform signature verification
// TODO: allow clients to sign the service registration request
int main(int argc, char* argv[]){
    Config config;
    Sock server;
    server.listen(config.port);
    uint16_t port = server.getPort();
    if(port != config.port){
        config.port = port;
        config.save();
    }

    while(true){
        if(select()){
            // accept connection and process one of 2 types of requests:
            // POST when a new message arrives or GET when GUI requests an update
            // POST uploads binary data which gets processed and put into a JSON message list
            // some posts are sent as notification to "tray app"
            // GET can get a file from /data/ dir or JSON message list
            // TODO: need messages for creating lists, moving contacts to friend list, marking msgs "read" etc.
        }
        //  pull updates from outnet if t > blah
    }
}