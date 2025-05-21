import net from "net"

net.createServer(socket => {
    socket.on("data", (msg) => {
        const [type, ...data] = msg.toString("utf-8").split("\n");

        if (type == "init") {
            socket.write("fillScreen\n20000");
        }
        if (type == "click") {
            socket.write("fillScreen\n" + Math.floor(Math.random() * 50_000));
        }
    });
}).listen(25279);