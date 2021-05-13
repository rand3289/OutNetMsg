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


function onLoad(){ // on Page load
    getGroups();
    getSavedMessages();
    getMessages();
    getInvites();
    setTimeout(getMessages, 1000);
}


// TODO: switch to EventSource() way of polling
async function getMessages() {
    let mes = document.getElementById("Messages");
    let div = document.getElementById("Chat");
    let display = div.style.display == "block";

    let data = await loadData(0,"");
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

    if(elemID == "Chat"){
        let user = document.getElementById("userID");
        user.innerHTML = global.lastKey;
    }
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


function sendMsgClick(){
    let user = global.lastKey;
    let txtArea = document.getElementById("msg");
    if(txtArea.value.length < 1){ return; }
    let msg = {type: 3, key: user, msg: txtArea.value };
    console.log("Sending a message: " + txtArea.value + " to user: " + user);
    txtArea.value = "";
    storeData(msg);
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


function keyClick(event){
    let key = event.target.text();
    console.log("Key clicked: "+ key);
    global.lastKey = key;
}


function groupClick(event){
    let group = event.target.text();
    console.log("Group clicked: "+ group);
    global.lastGroup = group;
}


function switchClick() { // button < > was clicked

}