#include "sock.h"
#include "utils.h"
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>
using namespace std;
namespace fs=std::filesystem;


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
    string fullPath = "./msgweb";
    fullPath += file;           // TODO: do not allow directories or sanitize path

    string ext = fs::path(file).extension().string();
    string mime = getMime(ext);

    ifstream f(fullPath, ios::binary);
    if( !f || ! fs::is_regular_file(fullPath) ){ // damn ifstream does not tell you if it's a directory
        writeStatus(conn, 404, "NOT FOUND");
        return false;
    }

    f.seekg(0, ios::end);
    size_t fsize = f.tellg();
    f.seekg(0, std::ios::beg);
    cout << "sending file: " << fullPath << " (" << fsize << " bytes)" << endl;

    stringstream ssh; // ss header
    ssh << "HTTP/1.1 200 OK\r\n"; // second "\r\n" separates headers from html
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
    auto it = mimeTypes.find(extension);
    if( end(mimeTypes) != it ){
        return it->second;
    }
    static string bin = "application/octet-stream"; // default mime type (browser offers to save file)
    return bin;
}
