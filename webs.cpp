#include "sock.h"
#include <cstring> // strlen()
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>
using namespace std;
using namespace std::filesystem;


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
        if( 0==strcmp(end,needle) ){ return end; }
    }
    return nullptr;
}


string getMime(const string& extension) {
    return "text/html"; // "application/octet-stream";
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
    string fullPath = "./data"; // TODO: do not allow directories or sanitize path
    fullPath += file;

    string ext = path(file).extension();
    string mime = getMime(ext);

    ifstream f(fullPath, ios::binary);
    if(!f){ 
        writeStatus(conn, 404, "NOT FOUND");
        return false;
    } // TODO: 404 not found
    f.seekg(0, ios::end);
    size_t fsize = f.tellg();
    f.seekg(0, std::ios::beg);
    cout << "sending file: " << fullPath << " (" << fsize << " bytes)" << endl;

    stringstream ssh; // ss header
    ssh << "HTTP/1.1 200 OK\r\n"; // second "\r\n" separates headers from html
//    ssh << "Content-Type: text/html\r\n";
    ssh << "Content-Type: "<< mime << "\r\n"; // ex: application/octet-stream
    ssh << "Content-Length: " << fsize << "\r\n";
    ssh << "\r\n";
    conn.write( ssh.str().c_str(), ssh.str().length() );

    char buff[1024*1024];
    while( !f.eof() ){
        f.read(buff, sizeof(buff) );
        size_t size = f.gcount();
        conn.write(buff, size);
    }
    return true;
}
