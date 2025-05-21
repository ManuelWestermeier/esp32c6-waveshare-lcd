import net from "net";

net.createServer(socket => {
    socket.on("data", (msg) => {
        const [type, ...data] = msg.toString("utf-8").split("\n");

        console.log("Received:", type, data);

        if (type === "init") {
            socket.write("fillScreen\n20000\n");
        } else if (type === "click") {
            const randomColor = Math.floor(Math.random() * 50000);
            socket.write("fillScreen\n" + randomColor + "\n");
        }
    });
}).listen(25279, () => {
    console.log("TCP server listening on port 25279");
});
