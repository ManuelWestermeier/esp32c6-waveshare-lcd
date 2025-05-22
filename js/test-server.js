import createServer from "./lib/server.js";

createServer((sock) => {
    sock.fillScreen(0x03b1);
    sock.setTextColor(0xe7bf);
    sock.setCursor(10, 10);
    sock.printText("Ready.");

    sock.onclick = async () => {
        const name = await sock.askText("Your name?", "Anonymous");
        sock.fillScreen(0x0000);
        sock.setCursor(0, 0);
        sock.printText("Hi " + name);
    };

    sock.ondbclick = async () => {

    };
}, 25279);