import net from "net";

net.createServer(socket => {
    console.log("Client connected");

    socket.on("data", (msg) => {
        const [type, ...data] = msg.toString("utf-8").trim().split("\n");

        console.log("Received:", type, data);

        if (type === "init") {
            // Respond with green screen (color code 0x07E0)
            socket.write("fillScreen\n2016\n"); // 2016 = 0x07E0
        } else if (type === "click") {
            const randomColor = Math.floor(Math.random() * 65535);
            socket.write("fillScreen\n" + randomColor + "\n");
        } else if (type === "dblclick") {
            socket.write("setCursor\n10\n50\n");
            socket.write("setTextColor\n65535\n"); // White
            socket.write("write\nDouble click!\n");
        } else if (type === "tripleclick") {
            socket.write("setCursor\n0\n100\n");
            socket.write("setTextColor\n63488\n"); // Red
            socket.write("write\nTriple click = restart\n");
        }
    });

    socket.on("end", () => {
        console.log("Client disconnected");
    });

    socket.on("error", err => {
        console.error("Socket error:", err);
    });

}).listen(25279, () => {
    console.log("TCP server listening on port 25279");
});
