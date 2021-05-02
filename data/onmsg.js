// This file is part of OutNet Messenger  https://github.com/rand3289/OutNetMsg


// https://stackoverflow.com/questions/12460378/how-to-get-json-from-url-in-javascript
async function loadView( view ) {
    let url = '/?info='+view;
    let obj = null;

    try {
        obj = await (await fetch(url)).json();
    } catch(e) {
        console.log('error: ' + e);
    }

    console.log(obj);
    return obj;
}


async function loadData(){
    let data = await loadView(0);
    let text = "ALL messages:<br>";
    console.log("data.lenght: " + data.length);
    for( i=0; i < (data.length-1); ++i){ // -1 because the last item is empty
        let msg = data[i].msg;
        text += msg + "<br>";
    }
    var div = document.getElementById("left");
    div.innerHTML = text;
}


function outNetMsgLoaded(){
    loadData();
}


function chatClicked(){ window.alert("chat"); }
function requestClicked() {  window.alert("request"); }
function groupsClicked() {  window.alert("groups"); }
function findUserClicked() {  window.alert("find user"); }
function textSearchClicked() {  window.alert("text search"); }
function settingsClicked() {  window.alert("settings"); }
