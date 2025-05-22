import createServer from "./lib/server.js";

createServer((client) => {
  function render() {
    client.fillScreen(0x18c3); // Slate blue background

    // Header bar
    client.fillRect(0, 0, 172, 30, 0x5acb); // Teal
    client.setTextColor(0xffff); // White text
    client.setCursor(10, 8);
    client.setTextSize(1);
    client.print("Smart Panel");

    // Main message area
    client.setCursor(10, 50);
    client.setTextColor(0xffff); // Ivory
    client.setTextSize(2);
    client.print("Hello!");

    // Footer hints
    client.fillRect(0, 290, 172, 30, 0xfd20); // Coral
    client.setCursor(10, 295);
    client.setTextColor(0x0000); // Black text
    client.setTextSize(1);
    client.print("Tap | Double-tap | Hold");
  }

  client.oninit = render;
  client.onrerender = render;

  // Tap → Yes/No
  client.onclick = async () => {
    const response = await client.askOk("Proceed?");
    client.fillRect(0, 100, 172, 30, 0x18c3); // Clear with bg color
    client.setCursor(10, 105);
    client.setTextColor(0xffe0); // Yellow
    client.setTextSize(2);
    client.print(response ? "Yes" : "No");
  };

  // Double-tap → Options
  client.ondbclick = async () => {
    const option = await client.askSelect(["1", "2", "3"]);
    client.fillRect(0, 140, 172, 30, 0x18c3);
    client.setCursor(10, 145);
    client.setTextColor(0xf81f); // Pink
    client.setTextSize(1);
    client.print("Selected: " + option);
  };

  // Long-press → Ask for search term
  client.onlongclick = async () => {
    const term = await client.askText("Search?");
    if (!term) return;

    // Simulated "search" response
    client.fillRect(0, 190, 172, 50, 0x18c3);
    client.setCursor(10, 195);
    client.setTextColor(0x07ff); // Cyan
    client.setTextSize(1);
    client.print("Searching: " + term);

    // Simulate web search by showing a fake result
    setTimeout(() => {
      client.fillRect(0, 240, 172, 40, 0x18c3);
      client.setCursor(10, 245);
      client.setTextColor(0xffff); // White
      client.setTextSize(1);
      client.print("Result: " + term + ".com");
    }, 1500);
  };
}, 25279);
