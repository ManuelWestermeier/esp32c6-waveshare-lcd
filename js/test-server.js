import createServer from "./lib/server.js";

createServer((client) => {
    var colors = [];
    var index = 0;

    function render() {
        client.setTextColor((1 << 16) - 1);
        client.fillScreen(colors[index]);

        client.setTextSize(15);
        client.setCursor(50, 100);
        client.printText(index + 1);
    }

    client.oninit = render;
    client.onrerender = render;

    client.onclick = () => {
        if (++index == colors.length) index = 0;
        render();
    }

    client.ondbclick = async () => {
        client.fillScreen(parseInt(await client.askText("color")))
    }
}, 25279);