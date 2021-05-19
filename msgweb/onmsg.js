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


function storeData(obj){ // send data to server via HTTP POST
    let req = new XMLHttpRequest();
    req.open("POST", "/", true);
    req.setRequestHeader('Content-Type', 'application/json');
    let jsn = JSON.stringify(obj);
    console.log("Sending JSON: " + jsn);
    req.send( jsn );
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


async function getGroups(){ // get a list of groups and all keys in those groups.  list them under "Groups div"
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

    setTimeout(getMessages, 2000);
}


// show global.groups on the screen (in 3 DIVs)
function showGroupsAndUsers(){
    let groups = document.getElementById("Groups");
    groups.innerHTML = "";
    for (const [grp, keys] of globals.groups){
        if(grp == "Friends") { continue; } // Friends go in Friends DIV (see below)
        groups.innerHTML += "<div onclick='groupClick(this)' id=" + grp + ">" + grp;
        for(const key of keys){
            groups.innerHTML += "<div align='right' id=" +key+ ">" +key+ "</div>";
        }
        groups.innerHTML += "</div>"; // group div
    }

    // Friends group is treated different since user keys do not go under the group name in UI
    let friends = document.getElementById("Friends"); // now load friends
    friends.innerHTML = "";
    for(const key of globals.groups.get("Friends").values() ){
        friends.innerHTML += "<div onclick='keyClick(this)' id=" +key+ ">" +key+ "</div>";
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
        all.innerHTML += "<div id=" +key+ " onclick=keyAddClick(this)>" +key+ "</div>";
    }
}


function keyAddClick(key){ // add key to group by clicking on a key
    if(globals.lastGroup.length > 0){
        let grp = globals.groups.get(globals.lastGroup);
        grp.add(key.id);
        console.log("Added " + key.id + " to group " +globals.lastGroup);
    }
    showGroupsAndUsers();
}


function tabBtnClick(event, elemID){ // a button that switches tabs on the right side is clicked
    let tabs = document.getElementsByClassName("tabs");
    for(let i=0; i < tabs.length; ++i){
        tabs[i].style.display = "none";
    }
    console.log("tab: "+ elemID);
    let div = document.getElementById(elemID)
    div.style.display = "block";
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

    let butn = document.getElementById("MsgButton");
    butn.disabled = true;

    let userLabel = document.getElementById("UserID");
    userLabel.scrollIntoView();
}


async function findUserClick() { // user is trying to find a publick key by providing a par ot it
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


async function addGroupClick() { // user is adding a "message group"
    let grpsList = document.getElementById("Groups");
    let field = document.getElementById("GroupName");
    let group = field.value;
    globals.groups.set(group, new Set() );
    let obj = { type: CMD.GRP_CREATE, grp: group };
    let ret = await storeData(obj);
    // TODO: add error message if ret is not 200
    grpsList.innerHTML += "<div onclick='groupClick(this)' id="+group+">" + group + "</div>";
    field.value = "";
}


// TODO: change this whole thing to "lastElement.id" ???
// TODO: merge with groupClick() ???
function keyClick(keyDiv){ // user clicked on a public key
    if(globals.lastElement){
        globals.lastElement.style.backgroundColor = "";
    }
    globals.lastElement = keyDiv;
    globals.lastElement.style.backgroundColor = "yellow";

    let key = keyDiv.id;
    console.log("selected key "+ key);
    globals.lastKey = key;
    globals.lastGroup = ""; // clear the group if key is selected

    let userLabel = document.getElementById("UserID");
    userLabel.innerHTML = key;
}


function groupClick(groupDiv){ // user clicked on a "message group"
    if(globals.lastElement){
        globals.lastElement.style.backgroundColor = "";
    }
    globals.lastElement = groupDiv;
    globals.lastElement.style.backgroundColor = "yellow";

    let group = groupDiv.id;
    console.log("selected group "+ group);
    globals.lastGroup = group;
    globals.lastKey = ""; // clear the key if group is selected

    let userLabel = document.getElementById("UserID");
    userLabel.innerHTML = group;
}


function msgTyped(){ // textarea with id Msg changed
    let msgbut = document.getElementById("MsgButton");
    let txta = document.getElementById("Msg");
    msgbut.disabled = txta.value.length <= 0; // enable if len > 0
    if(globals.lastElement == undefined){ // make sure global."user name" is set
        msgbut.disabled = true;           // otherwise who are we sending a message to?
    }
}
