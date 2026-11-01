# Machine definition files

> **Status: proposal.** This describes how word definitions, canned cycles and check settings should be organized once there are many machines. Today they live in two shared files (`data/gcode-words.json` and `data/gcode-rules.json`); the migration plan is at the end.

## Summary

- **One file per machine, built in layers.** A machine file names the files it builds on (`extends`) and only holds what's different. The layers are: a process base (mill, lathe, router, laser, wire EDM…), a control or brand (Fanuc, Haas, Okuma…), an optional model (Haas VF-2), and the shop's own machine.
- **Everything about a machine goes in its file:** the English words, address letters and number format, modal groups, canned cycles and their help text, subprogram calls, machine options (travel, live tooling, Y axis), and which checks are on and how severe they are.
- **The check logic and its wording stay in the app.** "Two codes from one group" works the same on every control; what differs is which codes are in which group. So the rule code and messages are shared, and the tables the rules read come from the machine file.
- **Private or shared is decided by where the file lives.** Built-in files ship with GlossCode, shop files live in a shared folder the shop picks (a network drive), and personal files live in the user's settings folder. GlossCode never uploads anything. Sharing means exporting a file on purpose.
- **Codes can have a decimal part** (`G43.4`, `G05.1`, `A00.0`). The code key keeps the decimal, so `G43.4` and `G43` are different codes, while `G1`, `G01` and `G001` are the same.
- **A GUI editor is worth building**, on top of plain JSON files that stay readable and diffable. It shows each value's layer, writes only to the user's own file, and checks widths and duplicates as you type.

## Layers

```
process base      base.mill            base.lathe          base.wire-edm   …
      │                │                    │
control/brand     fanuc.mill           fanuc.lathe
      │                │                    │
                  haas.mill            haas.lathe          (both also extend haas.common)
      │                │
model (optional)  haas.vf2
      │                │
shop machine      acme.vf2-cell3       (private, in the shop folder)
```

A file's `extends` is a list. Files are applied in order, then the file itself goes on top. That lets a brand share pieces between its mill and lathe files: `haas.mill` extends `["fanuc.mill", "haas.common"]`, where `haas.common` holds M97 local subprograms and the other Haas codes both machine types use.

| Layer | Holds | Examples |
|---|---|---|
| `process` | What every machine of this kind has: address meanings (X, Y, Z, U, W, I, J, K…), the core G and M codes, modal groups, the "process on" codes (spindle, beam, torch, wire), which checks apply | `base.common`, `base.mill`, `base.lathe`, `base.mill-turn`, `base.router`, `base.laser`, `base.plasma`, `base.waterjet`, `base.wire-edm`, `base.sinker-edm` |
| `control` | A control family's dialect: its cycles and their words, sub calls, foreign codes, number format | `fanuc.mill`, `haas.lathe`, `okuma.mill`, `grbl.router`, `linuxcnc.mill` |
| `model` | One model's facts: travel, max RPM, tool count, standard options | `haas.vf2`, `okuma.lb3000` |
| `machine` | One real machine in one shop: its options, in-house M-codes, custom macros, check settings | `acme.vf2-cell3` |

The process bases for router, laser, plasma, waterjet and EDM are starting points. Their codes vary much more between builders than on mills and lathes, so they ship `verified: false` and get a brand layer as real programs come in.

### Merging

The same rules the rules file uses today:

- Objects merge key by key, so a machine file that sets `codes.M88.long` keeps the inherited `short` and `desc`.
- Arrays replace (a modal group listed in a file is the whole group).
- `null` removes an inherited entry (`"G87": null` on Haas, which uses G77 for back boring).
- The resolved machine is what the editor uses. The GUI editor shows which file each value came from.

### Where files live

| Folder | Who writes it | Shared? |
|---|---|---|
| `machines/` beside the program | GlossCode releases | Built in, read-only |
| Shop folder (set in Settings, can be a network path) | The shop | With everyone who points at that folder |
| `%APPDATA%\GlossCode\machines` (`~/.config/glosscode/machines` on Linux) | The user | No |

Ids are unique across all three. A shop or personal file that uses the same id as a built-in file replaces it, which lets a shop fix a mistake in a built-in table without waiting for a release; the editor marks the machine "modified built-in" so it isn't a surprise. The usual way is to extend, not replace.

### Private and shared

Location is what keeps a file private: nothing in a personal or shop folder leaves the computer or network unless someone exports it. A file can also say `"private": true`. That doesn't change how it loads; it makes **Export** ask before writing it out, and makes Export leave out the `notes` and `shop` sections (asset numbers, cell names, who to call). Export offers two forms:

- **This layer only**: the small file with just the changes. The person receiving it needs the same base files, which are listed in `extends`.
- **Flattened**: every inherited value written into one file with `extends: []`, for someone on a different GlossCode version or with no base files.

Import copies the file into the personal folder and lists any `extends` it can't find.

### Definitions written from manuals

Most built-in tables will be checked against builders' programming manuals. Manuals are copyrighted, and some come with license terms or confidentiality notices, so these rules apply:

- **Manuals never go in the repository.** Keep the PDFs on your own computer or the shop's drive. `manuals/` is in `.gitignore` as a local place for them.
- **Write facts in our own words.** A definition records what a code does, which words it takes and which modal group it's in. Codes and their meanings are facts. Descriptions, cycle help and examples are written fresh and kept short, never copied from the manual. Never copy tables, drawings or examples from a manual.
- **Record the source without quoting it.** `"source": { "manual": "Haas Mill Operator's Manual", "revision": "2024", "section": "G-codes" }` says where to look. `verified: true` means someone checked the file against that source.
- **When in doubt, keep it private.** If a manual's terms forbid sharing what's in it (an NDA, or a "confidential" or "for the licensed user only" notice), or nobody is sure, the definition goes somewhere private rather than in the built-in files:
  - the personal folder or the shop folder, which never leave the computer or the network;
  - or a **separate private git repository** (for example `glosscode-machines-private`) that the shop folder points at, for a team that wants history and review;
  - `machines/private/` in a local checkout of this repository, which is also in `.gitignore`, for trying a definition out before deciding where it goes.

Whether a given definition can be published is a legal question this document can't answer. The defaults above keep the safe choice easy: private unless someone decides otherwise.

## The file

One JSON file per layer. Keys that aren't needed are left out. A full example of each layer is in [`machine-definitions/examples/`](machine-definitions/examples/).

```jsonc
{
  "glosscode": "machine/1",          // format version
  "id": "acme.vf2-cell3",            // unique; lowercase, dots and dashes
  "name": "Haas VF-2 (cell 3)",
  "layer": "machine",                // process | control | model | machine
  "extends": ["haas.vf2"],
  "private": true,
  "verified": true,                  // false = written from general knowledge
  "author": "Acme Machine, programming group",

  "machine": {                       // what the editor and the previews need
    "process": "mill",               // inherited from the process base; shown here for clarity
    "control": "Haas NGC",
    "units": "inch",
    "travel": { "X": 30, "Y": 16, "Z": 20 },
    "maxRpm": 8100, "maxFeed": 650, "tools": 20,
    "eob": false,
    "options": { "rotary": "A", "tsc": true, "probe": true }
  },

  "addresses": {                     // what each letter means on this machine
    "A": { "role": "axis", "rotary": true, "long": "A", "short": "A" }
  },

  "numbers": { "noDecimal": "leastIncrement", "increment": { "inch": 0.0001, "mm": 0.001 } },

  "codes": {                         // English words for code words (G, M and any other code letter)
    "M51": { "long": "VISEOPEN", "short": "VOPN", "desc": "Open the hydraulic vise (cell 3 wiring)" },
    "M52": { "long": "VISECLS",  "short": "VCLS", "desc": "Close the hydraulic vise" }
  },

  "modalGroups": { },
  "mConflicts": [["M51", "M52"]],
  "cycles": { },                     // words, help text, sketch and options per canned cycle
  "subCalls": { },
  "foreign": { },                    // codes that mean something else on this control

  "checks": {                        // turn rules off or change severity; see "Checks" below
    "rapid-below-zero": "off",
    "missing-decimal": "warning"
  },
  "lineRules": [ ],                  // simple in-house rules; see below

  "notes": "Vise valves on M51/M52 since the 2025 rewire.",
  "shop": { "asset": "VF2-0331", "cell": "3" }
}
```

What each section replaces today:

| Section | Comes from |
|---|---|
| `codes`, `addresses` | `gcode-words.json` `codes`, `address` and `machineOverrides` |
| `numbers`, `modalGroups`, `mConflicts`, `programEnds`, `toolLength`, `toolChange`, `dwellWords`, `subCalls`, `subReturn`, `arcTolerance`, `feedPlausible`, `speedClamp`, `diameterX`, `wordHints`, `wordMeaning`, `foreign`, `referenceOnly`, `keywords` | `gcode-rules.json` `dialects.<control>.<mill or lathe>` |
| `cycles` | `dialects.<control>.<type>.cycles` and `cycleHelp`, joined into one entry per cycle |
| `machine` | the mockup's `MACHINES` profiles |
| `checks` | new (the severity override `checks.md` already promises) |

Because each file is for one process, the `mill` / `lathe` sub-entries in today's dictionary go away: `G50` is scaling-off in `base.mill` and the RPM clamp in `base.lathe`.

### Codes with a decimal part

Many controls have codes with a decimal part, such as `G90.1`/`G91.1` (arc center distance mode), `G54.1 P1` (extra work offsets) and `G41.1`/`G42.1` (LinuxCNC cutter comp with a diameter word). A machine could also write every code with one decimal place (`G00.0`, `A00.0`). The file format and the parser handle all of these. The built-in files only include a decimal code once it has been checked against that control's manual, so anything else is left for users to add in their own machine files.

- A letter's `role` says whether a number after it is a **value** (`axis`, `feed`, `speed`, `offset`, `count`, `time`…) or picks a **code** (`code`). G and M are `code` in `base.common`. A machine where A selects a function rather than a rotary axis sets `"A": { "role": "code" }`, and its `codes` table gets `A00.0`-style entries.
- **Code keys** are the letter, the whole number padded to two digits, and the decimal part if it isn't zero: `G01`, `G43.4`, `A12.5`. When reading a program, `G1`, `G01`, `G001`, `G1.` and `G01.0` all find `G01`; `G43.4` only finds `G43.4`. An unknown `G43.4` is never shown as `G43`; it gets `unknown-word`.
- A control where `.0` is a different code from no decimal (rare) sets `"exactDecimal": true` on that letter, and then `G01` and `G01.0` are separate keys.
- `write` says how GlossCode writes codes it inserts (autocomplete, Insert menu, Renumber). `"G": { "role": "code", "write": "00.0" }` makes autocomplete insert `G00.0`. Words already in the program keep the form they were typed in.
- For value letters, `numbers` keeps working the way it does today (a distance with no decimal point reads as the smallest increment on Fanuc-style controls).
- A decimal code goes in `modalGroups` like any other code, so `G90.1` and `G91.1` form their own group, and `G59.1` can join the work offset group.
- A code that takes a selector word (`G54.1 P3`, Haas `G154 P12`) lists the word in `wordHints` as `requires`, the same way `G43` lists `H`.

Candidates for the built-in files, from a reference table. None of these have been checked against a manual yet, so they stay out of the built-in files until they are:

| Control | Decimal codes |
|---|---|
| Fanuc (0i, 30i/31i/32i) | G90.1 / G91.1 (arc IJK distance), G54.1 P1–P48 (extended work offsets, option) |
| Haas NGC | G90.1 / G91.1. Extended offsets use G154 P1–P99, not G54.1 |
| Hurco WinMax | G51.1 (mirroring), G54.1 (auxiliary work offsets), G94.1 (rotary tangential velocity) |
| LinuxCNC | G90.1 / G91.1, G41.1 / G42.1 (cutter comp with a D diameter word), G59.1 / G59.2 / G59.3 |
| Mach3 / Mach4 | G90.1 / G91.1, G59.1 / G59.2 / G59.3; Mach4 also G54.1 P1–P248 |
| Centroid | G90.1 / G91.1 (varies by CNC12 version). Extended fixtures are G54 P1–P12, with no decimal |
| GRBL | G91.1 only (incremental arc IJK is the default; G90.1 isn't implemented) |

Word-style codes like Okuma `CALL`, `RTS` and `GOTO` stay in `keywords`, with their English in `codes`.

### Canned cycles

Each cycle is one entry that holds everything the checks, word hints, Insert menu and F1 guide need, so they can't disagree:

```jsonc
"cycles": {
  "G83": {
    "name": "Peck drill",
    "category": "drilling",           // Insert menu group
    "requires": ["Z", "R"],
    "choices": [["Q"], ["I", "J", "K"]],
    "optional": ["X", "Y", "F", "L", "P"],
    "feed": true, "spindle": true,
    "needs": [],                       // machine options, e.g. ["liveTooling"]; hidden when missing
    "sketch": "peck",
    "about": "Peck drilling. Each peck retracts to R to clear chips.",
    "words": {
      "Q": ["0.1", "Depth of each peck.", "PECK"],
      "I": ["0.2", "First peck depth (use I, J and K instead of Q).", "PECK1"]
    }
  }
}
```

A machine can add a cycle (a builder's own G-code), remove one with `null`, or change one word: a file with only `"G83": { "words": { "Q": ["0.05", "Peck depth in mm", "PECK"] } }` keeps the rest. Machine options that hide cycles and words (`liveTooling`, `yAxis` today) are declared in `machine.options`, and a cycle's `needs` lists the options it requires.

Help text is plain English and is the same in every layer unless a layer overrides it. Translations, later, would be separate files keyed by cycle and word, not more fields here.

### Column order

The **Columns by word type** layout reads its columns from `columns`, in order from left to right. The built-in order puts the most used words first: motion and canned cycle codes, X, Y, Z, the arc and cycle words, F, S, the spindle M-codes, the tool, coolant, then setup G-codes, other M-codes, rare words and the comment. A word goes in the first column that matches it, so a code listed in an earlier column (M03 under Spindle) never reaches a later catch-all (M).

```jsonc
"columns": [
  { "id": "MOT",  "label": "Motion",  "groups": ["motion", "canned cycle"], "cycles": true, "codes": ["G04"], "carry": "motion" },
  { "id": "X",    "label": "X",       "letters": ["X", "U"], "carry": "position" },
  { "id": "SPIN", "label": "Spindle", "codes": ["M03", "M04", "M05"], "carry": "spindle" },
  { "id": "G",    "label": "G setup", "letters": ["G"] },
  { "id": "OTHER","label": "Other",   "other": true },
  { "id": "NOTE", "label": "Comment", "comments": true }
]
```

- `letters`: any word starting with these letters. `codes`: these exact codes. `groups`: every code in these modal groups. `cycles`: every canned cycle the machine has.
- `carry`: what an empty cell shows, greyed out, when the value is still active from an earlier line: `motion` (the motion code or active cycle), `position` (where the axis was last sent), `cycle` (R and Q kept by an active canned cycle), `value` (the last F or S), `spindle`, `coolant` and `tool`. Columns with no `carry` stay empty.
- Like other arrays, a layer that sets `columns` replaces the whole list. Only columns a program uses are shown, so extra columns cost nothing.

## Checks: how much is machine-specific

There are 31 checks today. None of their logic needs to change per machine, and almost all of the data they read does.

| | Machine-specific? | Where it lives |
|---|---|---|
| The logic of each check (how to find a modal clash, an arc that doesn't close) | No | The core (`checks.c`; `checkProgram()` in the mockup) |
| Title, message and explanation | No | `data/checks.json` in the app (renamed from the `rules` half of `gcode-rules.json`) |
| On/off and severity | Yes | `checks` in the definition file |
| The tables the checks read | Yes | The definition file |

How the 31 break down:

- **9 are fully generic** and only use severity: `empty-value`, `arc-r-and-ijk`, `arc-no-center`, `arc-full-circle-r`, `comp-start-on-arc`, `cycle-left-on`, `comp-left-on`, `feed-mode-changed`, `no-feed-rate`.
- **19 read a table from the definition file**: modal groups (`modal-group-clash`), M conflicts, foreign codes, cycles (three checks), sub calls (`missing-label`), program ends, tool length codes (two checks), dwell words, number format (`missing-decimal`), the dictionary itself (`unknown-word`), arc tolerance, feed plausibility, the speed clamp code (`css-no-clamp`), which letters can repeat in a block (`dup-address`), `comp-no-d`, `g28-absolute`.
- **3 only make sense for some processes**: `spindle-off-feed`, `no-speed`, `rapid-below-zero`.

Two changes come out of this:

1. **"Spindle" becomes "process".** A laser has a beam, a plasma cutter a torch, a waterjet a pump and abrasive valve, a wire EDM machining power. Each process base declares `process: { "on": [...], "off": [...], "label": "beam" }`, and `spindle-off-feed` becomes "cutting with the beam off" on a laser without a separate rule. Process bases turn off what doesn't apply (`tlo-missing` on a laser, `css-no-clamp` on a mill).
2. **A small set of declarative rules for shops**, so in-house conventions don't need code. Each `lineRules` entry is a condition and a message, no scripting:

   ```jsonc
   "lineRules": [
     { "id": "acme.vise-before-start", "when": { "code": "M03" }, "requiresEarlier": ["M52"],
       "severity": "warning", "message": "Close the vise (M52) before starting the spindle." },
     { "id": "acme.t-with-m06", "when": { "code": "M06" }, "requiresOnLine": ["T"],
       "severity": "error", "message": "M06 needs a T word on the same line in this shop." }
   ]
   ```

   The forms to start with: `requiresOnLine`, `forbidsOnLine`, `requiresEarlier` (anywhere before in execution order) and `requiresActive` (a modal code that must be on). Anything more complex becomes a built-in check.

## A GUI editor

Yes, it would help. Most people adding a machine are changing a handful of M-codes and a cycle or two, and a form is easier than getting JSON brackets right. It grows out of the existing **Machine profiles** window:

- **General**: name, process, control, the layer it extends, travel and limits, options (live tooling, Y axis, rotary axes), end of block, private.
- **Words**: a searchable table of every code and address letter with long word, short word, description and the layer it came from. Edited cells are bold; **Reset** goes back to the inherited value. An **Only changed here** filter shows the file's own entries.
- **Cycles**: the list by category; each opens the same layout as the Insert dialog, with required, choice and optional words, help text and example values, and a live preview of the F1 guide.
- **Checks**: every rule with Off / Info / Warning / Error, the inherited setting next to it, and the `lineRules` builder.
- **Try it**: the current program re-checked and re-translated with the unsaved changes, so the effect is visible before saving.

The editor checks as you type: long words over 10 columns and short words over 8, two codes with the same English word (autocomplete needs them unique), malformed code keys, a cycle word listed as both required and optional, and `extends` that can't be found.

Two limits keep it safe. It only writes to personal or shop files, so editing a built-in machine starts with **Make my own copy** (a new file that extends it). And it writes the JSON with one entry per line in a stable key order, so a shop that keeps its machine files in git gets small, readable diffs.

## Migration

1. Add the loader with merge and `extends` in the core, and a converter script that splits today's two files into `machines/base.*`, `machines/fanuc.*`, `machines/haas.*` and `machines/okuma.*`. Check that the resolved Haas, Fanuc and Okuma mill and lathe definitions match today's tables exactly.
2. Move check titles and messages to `data/checks.json`, and point the mockup at the resolved definitions instead of `DICT` and `RULES` (the sync script embeds the resolved machines).
3. Add the `machine` section in place of the mockup's `MACHINES` list, and the shop and personal folders.
4. Add `checks`, `lineRules` and the `process` table.
5. The GUI editor.
6. Thin `verified: false` bases for router, laser, plasma, waterjet and EDM, then brand layers as real programs arrive.

Until step 1 lands, keep editing `data/gcode-words.json` and `data/gcode-rules.json` as today.
