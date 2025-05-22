import createServer from "./lib/server.js";

createServer((client) => {
    var colors = [0, 50_000, 30_000];
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

    client.ondbclick = () => {
        if (--index == -1) index = colors.length - 1;
        render();
    }
}, 25279);