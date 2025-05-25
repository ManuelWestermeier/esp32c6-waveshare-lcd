import axios from "axios";
import createServer from "./lib/server.js";

// Map display names to DuckDuckGo 'kl' codes
const languages = {
  English: "en-us",
  German: "de-de",
  French: "fr-fr",
  Spanish: "es-es",
  Italian: "it-it",
  Dutch: "nl-nl",
  Portuguese: "pt-pt",
};

async function query(search, kl) {
  try {
    const response = await axios.get("https://api.duckduckgo.com/", {
      params: {
        q: search,
        format: "json",
        no_redirect: 1,
        no_html: 1,
        kl, // Language-region format
      },
    });

    const data = response.data;
    if (data.AbstractText) return data.AbstractText;
    if (data.RelatedTopics?.length) {
      const firstTopic = data.RelatedTopics[0];
      if (typeof firstTopic === "object" && firstTopic.Text) {
        return firstTopic.Text;
      }
    }

    return "No result found.";
  } catch (err) {
    return "Error fetching results.";
  }
}

// Example CLI run
query("progrom", "de-de").then(console.log);

createServer(async (client) => {
  let search = "";
  let output = "Nothing yet...";
  let kl = "en-us"; // default language

  function render() {
    client.fillScreen(0);
    client.setTextColor(0xffff);

    client.setTextSize(1);
    client.setCursor(30, 8);
    client.print(search ? "LongPress to Delete..." : "Click to Search...");

    client.setTextSize(2);
    client.setCursor(0, 18);
    client.printText(`${search ? search + "?" : ""}\n\n${output}`, "");
  }

  client.onrender = render;

  client.oninit = async () => {
    const langNames = Object.keys(languages);
    const selectedIndex = await client.askSelect(langNames);
    kl = languages[langNames[selectedIndex]] || "en-us";
    render();
  };

  client.onclick = async () => {
    search = await client.askText("Search...", search);
    if (!search) return;

    output = "Searching...";
    render();

    const result = await query(search, kl);
    output = result.replaceAll("\n", " ");
    render();
  };
}, 25279);
