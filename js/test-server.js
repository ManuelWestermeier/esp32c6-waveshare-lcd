import createServer from "./lib/server.js";

createServer((client) => {
    client.fillScreen(0x03b1);
    client.setTextColor(0xe7bf);
    client.setCursor(10, 10);
    client.printText("Ready.");

    client.onclick = async () => {
        client.fillScreen(0x03b1);
    };

    client.ondbclick = async () => {

    };
}, 25279);