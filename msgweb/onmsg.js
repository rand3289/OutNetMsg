// This file is part of OutNet Messenger  https://github.com/rand3289/OutNetMsg
"use strict"; // helps detect errors


async function loadData( view, addParams ) { // get data from server via HTTP GET
    let url = '/?info='+view+addParams;
    let obj = null;
    let resp = null;

    try {
        resp = await fetch(url);
        obj  = await resp.json();
    } catch(e) {
        console.log('error: ' + e);
        console.log( resp );
    }

    console.log(obj); // TODO: comment this out since obejects may be large
    return obj; // return await obj; ???
}


async function storeData(obj){ // send data to server via HTTP POST
    let req = new XMLHttpRequest();
    req.open("POST", "/", true);
    req.setRequestHeader('Content-Type', 'application/json');
    let jsn = JSON.stringify(obj);
    console.log("Sending JSON: " + jsn);

    return new Promise (
        function(resolve) { // Fuck, JS is so backwards!
            req.onreadystatechange = function () { resolve(this.status); }
            req.send( jsn );
        }
    );
}


var globals = { // all "global" variables are stored here
    lastKey: "",
    lastGroup: "",
    lastElement: null,
    messages: [],
    groups: null, // map of string -> vector of strings
    invites: []
};


var INFO = {     // HTTP GET info requests
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


async function onLoad(){ // on Page load
    getGroups();
    getSavedMessages();
    getInvites();
    getMessages();
}


async function getGroups(){ // get a list of groups and all keys in those groups from the server
    let data = await loadData(INFO.grpList, "");
    globals.groups = new Map();
    for (const grp of data){
        globals.groups.set(grp, new Set() );
        let keys = await loadData(INFO.grpUsers, "&grp="+grp);
        for(const key of keys){
            globals.groups.get(grp).add(key);
        }
    }
    showGroupsAndUsers();
}


async function getSavedMessages(){ // get all messages for a single key / group
    // TODO: 
}


async function getInvites(){ // get all invitations from all users
    // TODO:
}


// TODO: switch to EventSource() way of polling
async function getMessages() { // get all new messages for all keys/ groups
    let data = await loadData(INFO.msgNew,"");
    setTimeout(getMessages, 2000);
    if( data == null ){ return; }

    let mes = document.getElementById("Messages");
    let div = document.getElementById("Chat");
    let display = div.style.display == "block"; // visible

    for(let i=0; i < data.length; ++i){
        let msg = data[i].msg;
        if(display && globals.lastKey == msg.key ){
            mes.innerHTML += "<div class='inMsg'>" + msg + "</div>";
            globals.messages[msg.key].push(msg);
        }
    }
}


// show global.groups on the screen (in 3 DIVs)
function showGroupsAndUsers(){
    let groups = document.getElementById("Groups");
    groups.innerHTML = "";
    for (const [grp, keys] of globals.groups){
        if(grp == "Friends") { continue; } // Friends go in Friends DIV (see below)
        groups.innerHTML += "<div class='clickable' onclick='groupClick(this)' id=" + grp + ">" + grp;
        for(const key of keys){
            groups.innerHTML += "<div align='right' id=" +key+ ">" +key+ "</div>";
        }
        groups.innerHTML += "</div>"; // group div
    }

    // Friends group is treated different since user keys do not go under the group name in UI
    let friends = document.getElementById("Friends"); // now load friends
    friends.innerHTML = "";
    for(const key of globals.groups.get("Friends").values() ){
        friends.innerHTML += "<div class='clickable' onclick='keyClick(this)' id=" +key+ ">" +key+ "</div>";
    }

    let allSet = new Set(); // build a set of ALL known keys to remove duplicates
    for(const gval of globals.groups.values() ){
        for(const gkey of gval){
            allSet.add(gkey);
        }
    }
    // put ALL users (public keys) into ALL users div for addition to other groups
    let all = document.getElementById("AllUsers");
    all.innerHTML = "";     // clear existing
    for(const key of allSet){
        all.innerHTML += "<div class='clickable' onclick=keyAddClick(this) id=" +key+ ">" +key+ "</div>";
    }

    let userLabel = document.getElementById("UserID"); // username next to SEND button
    userLabel.innerHTML = "";
}


function msgTyped(){ // textarea with id Msg changed
    let msgbut = document.getElementById("MsgButton");
    let txta = document.getElementById("Msg");
    msgbut.disabled = txta.value.length <= 0; // enable if len > 0
    if(globals.lastGroup == "" && globals.lastKey == ""){ // make sure global."user name" is set
        msgbut.disabled = true;           // otherwise who are we sending a message to?
    }
}
