#include "sock.h"
#include <cstring> // strlen
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
using namespace std;


string& ltrim(string& s){ // trim white spaces on the left
    auto ends = find_if(begin(s), end(s), [](char ch){ return !isspace(ch); }  );
    s.erase(begin(s), ends );
    return s;
}


string& rtrim(string& s){ // trim white spaces on the right
    auto start = find_if(rbegin(s), rend(s), [](char ch){ return !isspace(ch); } );
    s.erase( start.base(), end(s) );
    return s;
}


char* findReverse(char* haystack, char* needle){
    for(char* end = haystack+strlen(haystack)-strlen(needle); end >= haystack; --end) {
        if( 0==strstr(end,needle) ){ return end; }
    }
    return nullptr;
}


void writeStatus(Sock& conn, int status, const string& reason){ // 403 "DENIED"
    stringstream ss;
    ss << "HTTP/1.1 " << status << " " << reason << "\r\n\r\n";
    conn.write(ss.str().c_str(), ss.str().length() );
}


bool sendFile(Sock& conn, char* request){
    char * end = findReverse(request, (char*) "HTTP/1.1");
    if(nullptr == end){ // malformed HTTP request
        writeStatus(conn, 400, "BAD REQUEST");
        return false;
    }
    *end = 0; // null terminate the path

    string req = request;
    auto file = ltrim(rtrim(req));
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
