var ws;
$(function begin() {
    let protocol = document.location.protocol.replace('http', 'ws');
    ws = new WebSocket(protocol + '//' + document.location.host + '/chat', ['string', 'foo']);
    ws.onopen = function () {
        console.log('onopen');
    };
    ws.onclose = function () {
        console.log('onclose, reconnecting in one second');
        setTimeout(function () {
            ws = new WebSocket(protocol + '//' + document.location.host + '/chat', ['string', 'foo']);
        }, 1000);
    };
    ws.onmessage = function (message) {
        var urlCreator = window.URL || window.webkitURL;
        var blob = new Blob([message.data], {name: 'test.png', type: 'image/png'});
        var imageUrl = urlCreator.createObjectURL(blob);
        document.querySelector("img").src = imageUrl;
    };
    ws.onerror = function (error) {
        console.log("ERROR: " + error);
    };
});
