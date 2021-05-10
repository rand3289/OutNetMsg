## OutNet Messenger
OutNetMsg is an open source peer-to-peer messenger.  It is a REST service that runs on your computer with a web interface.  It uses <a href="https://github.com/rand3289/OutNet"> OutNet</a> service to find other users.  The messenger does NOT have any central server.  It does not belong to anyone or any company.  All data is stored on your machine.  All connections are direct with other users.  There are no third party servers.  OutNetMsg does not store anyone's data on your computer only messages addressed to you.

OutNetMsg service runs on linux or windows.  It can be accessed from a computer or a phone through any web browser.  Since this is a distributed service, users are encouraged to have it always running.

## Project Status
OutNetMsg is a work in progress. There is no stable version yet.  Latest builds can be found here: https://github.com/rand3289/OutNetBin

## Configuration
onmsg.cfg contains ALL OutNetMsg settings.  If it is missing, it is generated the first time executable onmsg runs.  You have to set "outIP" and "outPort" in order for OutNetMsg to be able to contact your local OutNet service and find it's peers.  OutNetMsg requires secretOutNet.key and publicOutNet.key to be in the same directory as the executable file.  They are used to sign outgoing messages.  *OutNet.key files are generated by OutNet service when it first runs.  OutNet will also log it's port number that has to be manually set in onmsg.cfg

## Internals
Messenger is written in C++.  To compile under linux or windows (using MinGW64 from https://www.msys2.org) type "make" in the project directory.  OutNetMsg does NOT have dependencies except OutNet and optional OutNetTray.  OutNetTray is used to display notifications when OutNetMsg is not open in the browser.  

./msgweb/ directory contains index.html and  onmsg.js that can be modified to change your messenger look and feel.  
./lib/ directory contains code borrowed from OutNet service plus json parser (json.hpp) and tweetnacl cryptographic library (tweetnacl.*)

## TODO:
* Implement all TODO: in the code
* Figure out the best way to run OutNetMsg as a system service (OS dependent). Write wrappers and leave console apps as is?
* Integrate OutNetMsg with OutNetTray app
* Discover OutNet without config file via SSDP