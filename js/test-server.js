import createServer from "./lib/server.js";

createServer((client) => {
  function render() {
    client.fillScreen(0);
    client.setTextColor(50000);
    client.setCursor(10, 10);
    client.setTextSize(2);
  }

  client.oninit = render;
  client.onrerender = render;

  client.onclick = async () => {
    console.log(await client.askOk(" Yes or No?"));
  };

  client.ondbclick = async () => {
    console.log(await client.askSelect(["1", "2", "3", "4"]));
  };

  client.onlongclick = async () => {
    console.log(await client.askText("Hi?"));
  };
}, 25279);
