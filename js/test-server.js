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

createServer(async (client) => {
  let search = "";
  let output = "Nothing yet...";
  let kl = "de-de"; // default language

  let textView = false;
  let textViewIndex = 0;

  function render() {
    client.fillScreen(0);

    if (textView) {
      client.setTextColor(0xffff);
      client.setCursor(0, 10);
      client.setTextSize(1);
      client.printText(output, " ");
      return;
    }

    client.setTextColor(30000);
    client.setTextSize(1);
    client.setCursor(30, 8);
    client.print(search ? "LongPress to Delete..." : "Click to Search...");

    client.setTextColor(50000);
    client.setTextSize(2);
    client.setCursor(0, 20);
    client.printText(`${search ? search + "?" : ""}\n\n${output}`, "");
  }

  client.onrerender = render;

  client.oninit = () => {
    render();
  };

  client.onlongclick = () => {
    search = "";
    output = "Nothing yet...";
    render();
  };

  client.ondbclick = async () => {
    const options = [
      "Change Language",
      "View All Text",
      "View Search",
      ...new Array(100).fill().map((_, i) => i + "hh"),
    ];
    const option = await client.askSelect(options);

    if (!options[option]) return;
    else if (option == options.indexOf("Change Language")) {
      const langNames = Object.keys(languages);
      const selectedIndex = await client.askSelect(langNames);
      kl = languages[langNames[selectedIndex]] || "en-us";
      render();
    } else if (option == options.indexOf("View All Text")) {
      textView = true;
      render();
    } else if (option == options.indexOf("View Search")) {
      textView = false;
      render();
    }
  };

  client.onclick = async () => {
    if (textView) {
      textViewIndex++;
      return render();
    }

    search = await client.askText("Search...", search);
    if (!search) return;

    output = "Searching...";
    render();

    const result = await query(search, kl);
    output = result.replaceAll("\n", " ");
    render();
  };
}, 25279);
