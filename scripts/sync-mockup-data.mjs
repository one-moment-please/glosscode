// Copies data/gcode-words.json and data/gcode-rules.json into the mockup,
// which embeds them so it works as a single file. Run from the repo root:
//   node scripts/sync-mockup-data.mjs
import { readFileSync, writeFileSync } from "node:fs";

const page = "docs/mockup/index.html";
let html = readFileSync(page, "utf8");
const embed = (name, file) => {
  const json = JSON.stringify(JSON.parse(readFileSync(file, "utf8")));
  const re = new RegExp(`^const ${name} = .*;$`, "m");
  if (!re.test(html)) throw new Error(`No "const ${name} = ...;" line in ${page}`);
  html = html.replace(re, () => `const ${name} = ${json};`);
};
embed("DICT", "data/gcode-words.json");
embed("RULES", "data/gcode-rules.json");
writeFileSync(page, html);
console.log("Updated", page);
