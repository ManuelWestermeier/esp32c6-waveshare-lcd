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

  print(text) {
    this.sendCommand("write", text);
  }

  println(text) {
    this.sendCommand("println", text);
  }

  printText(text, offsetChar = "") {
    text
      .split("\n")
      .forEach((t) => this.sendCommand("println", offsetChar + t));
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

  async askText(question, defaultValue = "") {
    this.sendCommand("ask-text", question, defaultValue);
    return (await this._waitFor("ask-text-value")).replaceAll("\\n", "\n");
  }

  async askOk(question) {
    this.sendCommand("ask-ok", question);
    return (await this._waitFor("ask-ok-value")) === "yes";
  }

  async askSelect(options) {
    this.sendCommand("ask-select", options.join("\n"), "::OPTIONS_END::");
    return parseInt(await this._waitFor("ask-select-value"), 10);
  }

  _waitFor(cmd) {
    return new Promise((resolve) => {
      this.listeners.set(cmd, resolve);
    });
  }

  _onData(chunk) {
    this.buffer += chunk;

    while (true) {
      const newlineIdx = this.buffer.indexOf("\n");
      if (newlineIdx === -1) break;

      const line = this.buffer.slice(0, newlineIdx).trim();
      this.buffer = this.buffer.slice(newlineIdx + 1);

      // Handle two-line responses like "ask-text-value\nvalue\n"
      if (this.listeners.has(line)) {
        const secondNewlineIdx = this.buffer.indexOf("\n");
        if (secondNewlineIdx === -1) {
          // Wait for the full value to arrive
          this.buffer = line + "\n" + this.buffer;
          break;
        }

        const val = this.buffer.slice(0, secondNewlineIdx).trim();
        this.buffer = this.buffer.slice(secondNewlineIdx + 1);

        const resolve = this.listeners.get(line);
        this.listeners.delete(line);
        resolve(val);
      } else {
        this._handleCommand(line);
      }
    }
  }

  _handleCommand(cmd) {
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

  server.listen(port);

  return server;
}
