var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
window.addEventListener('load', onload);
function onload(event) {
    initWebSocket();
}
function initWebSocket() {
    console.log('Trying to open a WebSocket connection');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}
function onOpen(event) {
    console.log('Connection opened');   
}
function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}
function onMessage(event) {
    var jsonReceived = JSON.parse(event.data);
    //set objects on webpage after receiving message from server
    D0.checked = jsonReceived["D0"];
    D1.checked = jsonReceived["D1"];
    D2.checked = jsonReceived["D2"];
    D3.checked = jsonReceived["D3"];
    D4.value = jsonReceived["D4"];
    A0.value = jsonReceived["A0"];
    console.log('Browser received message');
    console.log(event.data);
}
function sendMessage(event) {
    var JSONSend =
    {
        "D0" : D0.checked ,   
        "D1" : D1.checked ,
        "D4" : D4.value 
    }; 
    websocket.send(JSON.stringify(JSONSend));
    console.log('Browser sent message');
    console.log(JSON.stringify(JSONSend));
}


