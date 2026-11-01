# Checking GlossCode against a programming manual

Use this procedure whenever a programming manual for a control or machine arrives. Each review gets a file here named after the manual (`haas-vf-1995.md`). The manual itself never goes in the repository; see [machine-definitions.md](../machine-definitions.md#definitions-written-from-manuals).

## 1. Get readable text

- Put the PDF in `manuals/` (gitignored) or leave it in the project files.
- Try `pdftotext -layout`. Many archive manuals are scans with no text layer, so if that comes back nearly empty, OCR it: `pdftoppm -r 200 -gray -png` for each page, then `tesseract page.png out --psm 6`. Set `OMP_THREAD_LIMIT=1` and run four pages at a time, or tesseract crawls. A 420-page manual takes about 4 minutes.
- Find the offset between the PDF page and the manual's printed page number, so findings can cite the printed number.

## 2. Map the manual

Read the table of contents and sort every section into one of these:

| Priority | What to look for | Feeds |
|---|---|---|
| **High** | Address letter list (meanings, ranges, no-decimal behavior) | `numbers`, `addresses`, `missing-decimal` |
| **High** | G-code summary with modal groups and defaults | `modalGroups`, dictionary, `unknown-word`, `foreign` |
| **High** | Canned cycle pages (words, which are optional, alternatives) | `cycles`, `cycleHelp`, word hints |
| **High** | M-code summary (and how many M-codes a line can hold) | dictionary, `mConflicts`, `mPerBlock`, `programEnds` |
| **High** | Subprograms and local calls | `subCalls`, `subReturn`, `missing-label` |
| Medium | Dwell, units, work offsets, tool length and cutter comp details | `dwellWords`, `dwellNoDecimal`, `toolLength`, comp checks |
| Medium | Settings or parameters that change how codes behave | notes for the machine definition |
| Low | Macros, programming tutorials, example programs | later parser work; test programs |
| None | Conversational programming, operation, installation, electrical, parts | skip |

Write the map into the review file with page ranges.

## 3. Compare, table by table

Resolve the dialect the machine would use (base plus every layer it inherits), then check each item against the manual:

1. **Every G code in the summary.** Is it in the dictionary? Is its modal group right? Is anything in our tables missing from the manual (flag it in `foreign`, or remove it)?
2. **Every canned cycle.** Required words, optional words, alternatives (`choices`), whether it starts the spindle, and whether it's modal.
3. **Every M code.** Dictionary entry, conflicts, and the M-codes-per-line limit.
4. **Address letters.** What a number with no decimal point means for each letter, ranges, and letters with more than one meaning.
5. **Subprograms.** Call codes, what P names (an O program or an N label), repeat words, return codes.
6. **The checks.** Go through the rule list in [checks.md](../checks.md) and ask for each one whether this control behaves the way the rule assumes. G28, dwell units and feed modes are the usual surprises.

Note each difference with its page number. Mark anything the manual doesn't settle as unverified rather than guessing.

## 4. Record and apply

- Write the review file: the manual and revision, the map, what differs, what matches, and what's left out. Use our own words. No copied text, tables or drawings.
- Put the differences in the data. Today that means a dialect in `data/gcode-rules.json` (with `inherits`, `verified: true` and `source`), entries in `machineOverrides` in `data/gcode-words.json`, and `cycleHelp` text. Once the loader exists, it means a machine definition file. Run `node scripts/sync-mockup-data.mjs` afterwards.
- A difference that needs new check logic gets a data key (like `mPerBlock`) and a rule in `checks.md`, so other controls can use it.
- Try the result in the mockup on a short program that hits each difference, and on the old dialect to make sure nothing changed there.
- If the manual's terms say it's confidential, the definition goes somewhere private instead (see machine-definitions.md).
