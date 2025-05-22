import createServer from "./lib/server.js";

createServer((client) => {
    client.init = () => {
        client.fillScreen(0);
        client.setTextColor(50_000);

        client.setTextSize(4);
        client.printText("\nReady.");
    }

    client.onclick = () => {
        client.fillScreen(0x03b1);
    };

    client.ondbclick = () => {
        client.fillScreen(2577);
    };

    client.socket.onclose = console.log
}, 25279);