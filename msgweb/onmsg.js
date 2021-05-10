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
    return obj;
}


async function loadStartup(){
    let text = "ALL messages:<br>";
    let data = await loadData(0,"");
    console.log("data.lenght: " + data.length);
    for(let i=0; i < data.length; ++i){
        let msg = data[i].msg;
        text += msg + "<br>";
    }
    let div = document.getElementById("left");
    div.innerHTML = text;
}


function onLoad(){
    loadStartup();
}


function tabClick(event, elemID){
    let tabs = document.getElementsByClassName("tabs");
    for(let i=0; i < tabs.length; ++i){
        tabs[i].style.display = "none";
    }
    console.log("tab: "+ elemID);
    document.getElementById(elemID).style.display = "block";
    let tdRight = document.getElementById("tdRight");
    if(elemID == "Chat"){
        tdRight.style.verticalAlign = "bottom";
    } else {
        tdRight.style.verticalAlign = "top";
    }
}


function storeData(obj){
    let req = new XMLHttpRequest();
    req.open("POST", "/", true);
    req.setRequestHeader('Content-Type', 'application/json');
    let jsn = JSON.stringify(obj);
    console.log("Sending JSON: " + jsn);
    req.send( jsn );
}


function sendMsg(){
    let user = "FFFF"; // TODO: select which user
    let txtArea = document.getElementById("msg");
    if(txtArea.value.length < 1){ return; }
    let msg = {type: 3, key: user, msg: txtArea.value };
    console.log("Sending a message: " + txtArea.value + " to user: " + user);
    txtArea.value = "";
    storeData(msg);
}

function findUser(){
    let list = document.getElementById("UserList");
    let user = document.getElementById("UserKey").value;
    let data = loadData(4, "&user="+user);
    if(data.length == 0) {
        list.innerHTML = "Not found.";
    }
    else for(let i=0; i < data.length; ++i) {
        list.innerHTML += data[i] + "<BR>\r\n";
    }
}
