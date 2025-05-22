import createServer from "./lib/server.js";

createServer((client) => {
    client.fillScreen(0x03b1);
    client.setTextColor(0xe7bf);

    client.setCursor(100, 100);
    client.setTextSize(4);
    client.printText("Ready.");

    client.onclick = async () => {
        client.fillScreen(0x03b1);
    };

    client.ondbclick = async () => {
        client.fillScreen(2577);
    };
}, 25279);