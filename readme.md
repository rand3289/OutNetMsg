## OutNet Messenger
OutNetMsg is an open source peer-to-peer messenger.  It is a REST service that runs on your computer with a web interface.  It uses <a href="https://github.com/rand3289/OutNet"> OutNet</a> service to find other users.  The messenger does NOT have any central server.  It does not belong to anyone or any company.  All data is stored on your machine.  All connections are direct with other users.  There are no third party servers.  

OutNetMsg service runs on linux of windows.  It can be accessed from a computer or a phone through any web browser.

## Project Status
OutNetMsg is a work in progress. There is no stable version yet.  
To compile under linux or windows (using MinGW64 from https://www.msys2.org) type "make" in the project directory.  OutNetMsg does NOT have dependencies except OutNet and optional OutNetTray.  OutNetTray is used to display notifications when OutNetMsg is not open in the browser.

## Configuration
onmsg.cfg contains ALL OutNetMsg settings.  You have to set "outIP" and "outPort" in order for OutNetMsg to be able to contact your local OutNet service and find it's peers.  
If you want to modify the look and feel of the messenger, modify index.html and onmsg.js in the data directory.

## TODO:
* Implement all TODO: in the code
* Figure out the best way to run OutNetMsg as a system service (OS dependent). Write wrappers and leave console apps as is?
* Integrate OutNetMsg with OutNetTray app
* Discover OutNet without config file