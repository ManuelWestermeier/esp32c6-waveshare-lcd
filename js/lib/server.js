import net from "net";

export default function createServer(handler, port = 25279) {
    const server = net.createServer((socket) => {
        socket.setEncoding("utf-8");
        let buffer = "";
        const listeners = {};
        const sock = {
            onclick: null,
            ondbclick: null,
            sendCommand: (cmd, ...args) => {
                socket.write(cmd + "\n" + args.join("\n") + "\n");
            },
            fillScreen: (color) => sock.sendCommand("fillScreen", color),
            setCursor: (x, y) => sock.sendCommand("setCursor", x, y),
            printText: (text) => sock.sendCommand("write", text),
            drawPixel: (x, y, color) => sock.sendCommand("drawPixel", x, y, color),
            fillRect: (x, y, w, h, color) => sock.sendCommand("fillRect", x, y, w, h, color),
            setTextColor: (color) => sock.sendCommand("setTextColor", color),
            askText: async (question, def = "") => {
                sock.sendCommand("ask-text", question, def);
                return await waitFor("ask-text-value");
            },
            askOk: async (question, def = "") => {
                sock.sendCommand("ask-ok", question, def);
                return (await waitFor("ask-ok-value")) === "yes";
            },
            askSelect: async (options) => {
                options.forEach(opt => sock.sendCommand("ask-select", opt));
                sock.sendCommand("ask-select", "::OPTIONS_END::");
                return parseInt(await waitFor("ask-select-value"), 10);
            }
        };

        function waitFor(cmd) {
            return new Promise(resolve => {
                listeners[cmd] = resolve;
            });
        }

        socket.on("data", (chunk) => {
            buffer += chunk;
            let idx;
            while ((idx = buffer.indexOf("\n")) !== -1) {
                const line = buffer.slice(0, idx).trim();
                buffer = buffer.slice(idx + 1);

                handleCommand(line);
            }
        });

        function handleCommand(cmd) {
            if (cmd === "click" && typeof sock.onclick === "function") {
                sock.onclick();
            } if (cmd === "dblclick" && typeof sock.onclick === "function") {
                sock.ondbclick();
            } else if (listeners[cmd]) {
                waitBuffer = "";
                const resolve = listeners[cmd];
                delete listeners[cmd];
                let idx = buffer.indexOf("\n");
                if (idx === -1) return; // wait for full value
                const val = buffer.slice(0, idx).trim();
                buffer = buffer.slice(idx + 1);
                resolve(val);
            }
        }

        socket.on("error", () => { });
        socket.on("close", () => { });

        // Initial call for user-defined logic
        handler(sock);
    });

    server.listen(port, () => { });

    return server;
}