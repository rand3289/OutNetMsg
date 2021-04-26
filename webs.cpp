#include "sock.h"
#include <cstring> // strlen()
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>
using namespace std;
namespace fs=std::filesystem;


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
/******************************* end utility functions *****************************/


string& getMime(const string& extension);


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

    string ext = fs::path(file).extension();
    string mime = getMime(ext);

    ifstream f(fullPath, ios::binary);
    if(!f){ return false; } // TODO: 404 not found
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
    char buff[1024*1024];
    while( f.read(buff, sizeof(buff)) ){
        size_t size = f.gcount();
        conn.write(buff, size);
    }
    return true;
}


#include <map>
static map<string,string> mimeTypes = {
{".aac",	"audio/aac"},
{".arc",	"application/x-freearc"},
{".avi",	"video/x-msvideo"},
{".bin",	"application/octet-stream"},
{".bmp",	"image/bmp"},
{".bz",	"application/x-bzip"},
{".bz2",	"application/x-bzip2"},
{".css",	"text/css"},
{".csv",	"text/csv"},
{".doc",	"application/msword"},
{".docx",	"application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
{".epub",	"application/epub+zip"},
{".gz",	"application/gzip"},
{".gif",	"image/gif"},
{".htm",	"text/html"},
{".html",	"text/html"},
{".ico",	"image/vnd.microsoft.icon"},
{".ics",	"text/calendar"},
{".jar",	"application/java-archive"},
{".jpeg",	"image/jpeg"},
{".jpg",	"image/jpeg"},
{".js",	"text/javascript"},
{".json","application/json"},
{".mp3",	"audio/mpeg"},
{".mp4",	"video/mp4"},
{".mpeg",	"video/mpeg"},
{".odt",	"application/vnd.oasis.opendocument.text"},
{".oga",	"audio/ogg"},
{".ogv",	"video/ogg"},
{".png",	"image/png"},
{".pdf",	"application/pdf"},
{".rar",	"application/vnd.rar"},
{".rtf",	"application/rtf"},
{".svg",	"image/svg+xml"},
{".swf",	"application/x-shockwave-flash"},
{".tar",	"application/x-tar"},
{".tif",	"image/tiff"},
{".tiff",	"image/tiff"},
{".ts",		"video/mp2t"},
{".ttf",	"font/ttf"},
{".txt",	"text/plain"},
{".wav",	"audio/wav"},
{".weba",	"audio/webm"},
{".webm",	"video/webm"},
{".webp",	"image/webp"},
{".xhtml",	"application/xhtml+xml"},
{".xml",	"text/xml"},
{".zip",	"application/zip"},
{".3gp",	"video/3gpp"},
{".3g2",	"video/3gpp2"},
{".7z",	"application/x-7z-compressed"}
};


string& getMime(const string& extension) {
    static string bin = "application/octet-stream";
    auto it = mimeTypes.find(extension);
    if( end(mimeTypes) != it ){
        return it->second;
    }
    return bin;
}
