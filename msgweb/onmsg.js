// This file is part of OutNet Messenger  https://github.com/rand3289/OutNetMsg
"use strict"; // helps detect errors


// https://stackoverflow.com/questions/12460378/how-to-get-json-from-url-in-javascript
async function loadData( view, addParams ) {
    let url = '/?info='+view+addParams;
    let obj = null;
    let resp = null;

    try {
        resp = await fetch(url);
//        console.log( await resp.text() );
        obj  = await resp.json();
    } catch(e) {
        console.log('error: ' + e);
        console.log( resp );
    }

    console.log(obj); // TODO: comment this out since obejects may be large
    return obj; // return await obj; ???
}


function storeData(obj){
    let req = new XMLHttpRequest();
    req.open("POST", "/", true);
    req.setRequestHeader('Content-Type', 'application/json');
    let jsn = JSON.stringify(obj);
    console.log("Sending JSON: " + jsn);
    req.send( jsn );
}


var globals = {
    lastKey:"",
    lastGroup:"",
    messages: [],
    groups: [],
    invites: []
};


var INFO = {     // info requested by GUI via HTTP GET
    msgNew: 0,   // get ALL new messages
    msgUser: 1,  // get ALL messages for a user
    grpList: 2,  // get a list of groups
    grpUsers: 3, // get a list of users in a group
    findUser: 4, // find all keys containing a hex number
    findText: 5  // find all messages containing text
};


var CMD = {        // commands in HTTP POST request coming from GUI
    MSG_USER: 3,   // send a message to a user
    MSG_GROUP: 4,  // send a message to a group
    MSG_SEEN: 5,   // mark message read
    GRP_LEAVE: 6,  // group leave request
    GRP_CREATE: 7, // create a new group/list
    GRP_DELETE: 8, // delete a group
    GRP_ADD: 9,    // adding a user to a group/list
    GRP_RM: 10     // removing a user from a group/list
};


async function getGroups(){
    let groups = document.getElementById("Groups");
    let data = await loadData(INFO.grpList, "");
    for (const grp of data){
        groups.innerHTML += "<div onclick='groupClick(this)' id=" + grp + ">" + grp;
        let keys = await loadData(INFO.grpUsers, "&grp="+grp);
        for(const key of keys){
            globals.groups[grp].push(key);
            groups.innerHTML += "<div id="+key>+ ">" + key + "</div>";
        }
        groups.innerHTML += "</div>";
    }
}


async function onLoad(){ // on Page load
    getGroups();
    getSavedMessages();
    getMessages();
    getInvites();
    setTimeout(getMessages, 1000);
}

async function getSavedMessages(){
    // TODO: 
}
async function getInvites(){
    // TODO:
}


// TODO: switch to EventSource() way of polling
async function getMessages() {
    let mes = document.getElementById("Messages");
    let div = document.getElementById("Chat");
    let display = div.style.display == "block";

    let data = await loadData(INFO.msgNew,"");
    for(let i=0; i < data.length; ++i){
        let msg = data[i].msg;
        if(display && globals.lastKey == msg.key ){
            mes.innerHTML += "<div class='inMsg'>" + msg + "</div>";
            globals.messages[msg.key].push(msg);
        }
    }

    setTimeout(getMessages, 1000);
}


function tabClick(event, elemID){
    let tabs = document.getElementsByClassName("tabs");
    for(let i=0; i < tabs.length; ++i){
        tabs[i].style.display = "none";
    }
    console.log("tab: "+ elemID);
    let div = document.getElementById(elemID)
    div.style.display = "block";

    // bottom align chat within parent div
    // works for td, doesn't for div.  FUCK!!!
//    let right = document.getElementById("right");
//    if(elemID == "Chat"){
//        console.log("verticalAlign=bottom");
//        right.style.verticalAlign = "bottom";
//    } else {
//        right.style.verticalAlign = "top";
//    }
}


function sendMsgClick(){ // send a message to a user (key) or a group
    let user = globals.lastKey;
    if(user.length < 1){
        user = globals.lastGroup;
    }

    let txtArea = document.getElementById("Msg");
    let msgt = txtArea.value;
    if(msgt.length < 1){ return; }

    let msg = {type: 3, key: user, msg: msgt };
    console.log("Sending a message: " + msgt + " to user: " + user);
    storeData(msg);

    let msgDiv = document.getElementById("Messages");
    msgDiv.innerHTML += "<div class='outMsg'>" + msgt + "</div>";
    txtArea.value = "";
}


async function findUserClick() {
    let list = document.getElementById("UserList");
    list.innerHTML = "";

    let user = document.getElementById("UserKey").value;
    let data = await loadData(4, "&user="+user);

    if(data.keys.length == 0) {
        list.innerHTML = "Not found.";
    }
    else for(let i=0; i < data.keys.length; ++i) {
        list.innerHTML += data.keys[i] + "<BR>\r\n";
    }
}


function keyClick(keyDiv){
    let key = keyDiv.id;
    console.log("selected key "+ key);
    globals.lastKey = key;
    globals.lastGroup = ""; // clear the group if key is selected
    let userLabel = document.getElementById("UserID");
    userLabel.innerHTML = key;
    let butn = document.getElementById("MsgButton");
    butn.disabled = false;
}


function groupClick(groupDiv){
    let group = groupDiv.id;
    console.log("selected group "+ group);
    globals.lastGroup = group;
    globals.lastKey = ""; // clear the key if group is selected
    let userLabel = document.getElementById("UserID");
    userLabel.innerHTML = group;
    let butn = document.getElementById("MsgButton");
    butn.disabled = false;
}


async function addGroupClick() {
    let grpsList = document.getElementById("Groups");
    let field = document.getElementById("GroupName");
    let group = field.value;
    globals.groups[group];
    let obj = { type: CMD.GRP_CREATE, grp: group };
    let ret = await storeData(obj);
    // TODO: add error message if ret is not 200
    grpsList.innerHTML += "<div onclick='groupClick(this)' id="+group+">" + group + "</div>";
    field.value = "";
}


function switchClick() { // button < > was clicked
    alert("switch button clicked");
}