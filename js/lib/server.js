import net from "net";

class Client {
  socket;
  buffer = "";
  listeners = new Map();
  onclick = null;
  ondbclick = null;
  oninit = null;
  onrerender = null;
  onlongclick = null;

  constructor(socket) {
    this.socket = socket;
    socket.setEncoding("utf-8");

    socket.on("data", (chunk) => this._onData(chunk));
    socket.on("error", () => {});
    socket.on("close", () => {});

    // Bind event handlers so `this` works correctly
    this._onData = this._onData.bind(this);
    this._handleCommand = this._handleCommand.bind(this);
  }

  sendCommand(cmd, ...args) {
    this.socket.write(cmd + "\n" + args.join("\n") + "\n");
  }

  fillScreen(color) {
    this.sendCommand("fillScreen", color);
  }

  setCursor(x, y) {
    this.sendCommand("setCursor", x, y);
  }

  printText(text) {
    this.sendCommand("write", text);
  }
  print(text) {
    this.sendCommand("write", text);
  }

  drawPixel(x, y, color) {
    this.sendCommand("drawPixel", x, y, color);
  }

  fillRect(x, y, w, h, color) {
    this.sendCommand("fillRect", x, y, w, h, color);
  }

  setTextColor(color) {
    this.sendCommand("setTextColor", color);
  }

  setTextSize(size) {
    this.sendCommand("setTextSize", size);
  }

  async askText(question, _defautl = "") {
    this.sendCommand("ask-text", question, _defautl);
    return (await this._waitFor("ask-text-value")).replaceAll("\\n", "\n"); // Convert escaped \n to real newline
  }

  async askOk(question) {
    this.sendCommand("ask-ok", question);
    return (await this._waitFor("ask-ok-value")) === "yes";
  }

  async askSelect(options) {
    this.sendCommand(
      "ask-select",
      options.map((o) => o.replaceAll("\n", "\n")).join("\n"),
      "::OPTIONS_END::"
    );
    return parseInt(await this._waitFor("ask-select-value"), 10);
  }

  _waitFor(cmd) {
    return new Promise((resolve) => {
      this.listeners.set(cmd, resolve);
    });
  }

  _onData(chunk) {
    this.buffer += chunk;
    let idx;
    while ((idx = this.buffer.indexOf("\n")) !== -1) {
      const line = this.buffer.slice(0, idx).trim();
      this.buffer = this.buffer.slice(idx + 1);
      this._handleCommand(line);
    }
  }

  _handleCommand(cmd) {
    console.log("cmd", cmd);

    if (cmd === "click" && typeof this.onclick === "function") {
      this.onclick();
      return;
    }
    if (cmd === "dblclick" && typeof this.ondbclick === "function") {
      this.ondbclick();
      return;
    }
    if (cmd === "init" && typeof this.oninit === "function") {
      this.oninit();
      return;
    }
    if (cmd === "rerender" && typeof this.onrerender === "function") {
      this.onrerender();
      return;
    }
    if (cmd === "longclick" && typeof this.onlongclick === "function") {
      this.onlongclick();
      return;
    }

    if (this.listeners.has(cmd)) {
      // The next line after cmd is the value for the promise
      const idx = this.buffer.indexOf("\n");
      if (idx === -1) {
        // Wait for more data
        return;
      }
      const val = this.buffer.slice(0, idx).trim();
      this.buffer = this.buffer.slice(idx + 1);

      const resolve = this.listeners.get(cmd);
      this.listeners.delete(cmd);
      resolve(val);
    }
  }
}

export default function createServer(
  handler = (client = new Client()) => null,
  port = 25279
) {
  const server = net.createServer((socket) => {
    const client = new Client(socket);
    handler(client);
  });

  server.listen(port, () => {});

  return server;
}
