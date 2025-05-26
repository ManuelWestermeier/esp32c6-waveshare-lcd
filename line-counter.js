import path from "path";
import fs from "fs";

let lines = 0;

function read(pathName) {
  const isDir = fs.statSync(pathName).isDirectory();
  if (isDir) {
    const files = fs.readdirSync(pathName);
    for (const fileName of files) {
      read(path.join(pathName, fileName));
    }
  } else {
    if ([".ino", ".cpp", ".hpp", ".c", ".h"].includes(path.extname(pathName)))
      lines += fs.readFileSync(pathName).toString("utf-8").split("\n").length;
  }
}

const files = fs.readdirSync("./");

files.forEach(read);

console.log(lines);
