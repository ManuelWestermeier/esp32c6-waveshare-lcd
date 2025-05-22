import createServer from "./lib/server.js";

createServer((client) => {
    client.fillScreen(0x03b1);
    client.setTextColor(0xe7bf);
    client.setCursor(10, 10);
    client.printText("Ready.");

    client.onclick = async () => {
        const name = await client.askText("Your name?", "Anonymous");
        client.fillScreen(0x0000);
        client.setCursor(0, 0);
        client.printText("Hi " + name);
    };

    client.ondbclick = async () => {

    };
}, 25279);