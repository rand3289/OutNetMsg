// This file is part of OutNet Messenger  https://github.com/rand3289/OutNetMsg
// It countains all onclick event handlers
"use strict"; // helps detect errors


function keyDelClick(grp, key){  // delete a key (user) from a group
    console.log("Deleting key " +key+ " from group " +grp);
    let group = globals.groups.get(grp);
    group.delete(key); // TODO: notify user about deletion if it is your group / fail delete otherwise
    showGroupsAndUsers();
}


function grpDelClick(grp){
    globals.groups.delete(grp); // TODO: notify all userS about deletion if it is your group / fail delete otherwise
    showGroupsAndUsers();
}


async function keyAddClick(key){ // add key to group by clicking on a key
    let group = globals.lastGroup;
    if(group.length == 0){
        let ok = confirm("Add user to Friends?\r\nOtherwise click CANCEL and select a group to add to a group.");
        if(!ok){ return; }
        group= "Friends";
    }

    let addRequest = { type: CMD.GRP_ADD, grp: group, key: key.id };
    let ret = await storeData(addRequest);
    if(ret != 200){
        alert("Error adding user to group.");
        console.log("Error adding user to group. HTTP returned " + ret);
        return;
    }

    let grp = globals.groups.get(group);
    grp.add(key.id);
    console.log("Added " + key.id + " to group " +group);
    globals.lastKey = "";
    globals.lastGroup = "";
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
//    event.target.backgroundColor = 0xFFFF00;
}


async function sendMsgClick(){ // send a message to a user (key) or a group
    let user = globals.lastKey;
    if(user.length < 1){
        user = globals.lastGroup;
    }

    let txtArea = document.getElementById("Msg");
    let msgt = txtArea.value;
    if(msgt.length < 1){ return; }
    console.log("Sending a message: " + msgt + " to user: " + user);

    let msg = {type: 3, key: user, msg: msgt };
    let ret = await storeData(msg);
    if(ret != 200){
        alert("Error sending message to server.");
        console.log("Error sending message to server. HTTP returned " + ret);
        return;
    }

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
        let key = data.keys[i];
        list.innerHTML +="<div class='clickable' onclick=keyAddClick(this) id=" +key+ ">" +key.substring(0,15)+ "</div>";
    }
}


async function addGroupClick() { // user is adding a "message group"
    let field = document.getElementById("GroupName");
    let group = field.value;

    let obj = { type: CMD.GRP_CREATE, grp: group };
    let ret = await storeData(obj);
    if(ret != 200){
        alert("Error adding a group on the server.");
        console.log("Error adding a group on the server. HTTP returned " + ret);
        return;
    }

    globals.groups.set(group, new Set() );
    globals.lastGroup = ""; // TODO: set lastGroup to new group???
    globals.lastKey = "";
    field.value = "";
    showGroupsAndUsers();
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

    let userLabel = document.getElementById("UserID"); //  it's a <span>
    userLabel.innerHTML = group;
}
