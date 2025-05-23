import axios from "axios";
import createServer from "./lib/server.js";

async function querry(search) {
  try {
    const response = await axios.get("https://api.duckduckgo.com/", {
      params: {
        q: search,
        format: "json",
        no_redirect: 1,
        no_html: 1,
      },
    });

    const data = response.data;
    if (data.AbstractText) return data.AbstractText;
    if (data.RelatedTopics?.length)
      return data.RelatedTopics[0].Text || "No detailed result.";

    return "No result found.";
  } catch (err) {
    return "Error fetching results.";
  }
}

createServer(async (client) => {
  console.log("x", await client.askText("Search...", ""));

  let search = "";
  let output = "nothing yet...";

  function render() {
    client.fillScreen(0);
    client.setTextColor((1 << 16) - 1);

    if (!search || !output) {
      client.setTextSize(1);
      client.setCursor(30, 4);
      client.print("Click to Search...");
    }

    client.setTextSize(2);
    client.setCursor(0, 13);
    client.print(` ${search}\n ${output.replaceAll("\n", "\n ")}`);
  }

  client.onrerender = render;
  client.oninit = render;

  client.onclick = async () => {
    search = await client.askText("Search...", search);

    output = "Searching...";
    render();

    output = await querry(search);
    render();
  };
}, 25279);
