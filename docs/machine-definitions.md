# Machine definition files

> **Status: proposal.** This describes how word definitions, canned cycles and check settings should be organized once there are many machines. Today they live in two shared files (`data/gcode-words.json` and `data/gcode-rules.json`); the migration plan is at the end.

## Summary

- **Arranged around the control, like the manuals.** Control series › control model › machine type › machine model, with options as checkboxes: Haas › NGC › mill › VF-3SS, ☑ probing. The control decides the language; the machine model only holds hardware facts (travel, RPM, tool changer, gearbox) that turn some codes on or off.
- **Every level has a Generic choice**, worked out by GlossCode from the entries under it, which shows only what they all share and flags codes that aren't on every machine.
- **One file per node, built in layers.** Each file names the files it builds on (`extends`) and only holds what's different. Under the control tree sit the process bases (mill, lathe, router, laser, wire EDM…), and on top sits the shop's own machine.
- **Everything about a machine goes in its file:** the English words, address letters and number format, modal groups, canned cycles and their help text, subprogram calls, machine options (travel, live tooling, Y axis), and which checks are on and how severe they are.
- **The check logic and its wording stay in the app.** "Two codes from one group" works the same on every control; what differs is which codes are in which group. So the rule code and messages are shared, and the tables the rules read come from the machine file.
- **Private or shared is decided by where the file lives.** Built-in files ship with GlossCode, shop files live in a shared folder the shop picks (a network drive), and personal files live in the user's settings folder. GlossCode never uploads anything. Sharing means exporting a file on purpose.
- **Codes can have a decimal part** (`G43.4`, `G05.1`, `A00.0`). The code key keeps the decimal, so `G43.4` and `G43` are different codes, while `G1`, `G01` and `G001` are the same.
- **A GUI editor is worth building**, on top of plain JSON files that stay readable and diffable. It shows each value's layer, writes only to the user's own file, and checks widths and duplicates as you type.

## Layers

```
process base      base.mill                    base.lathe          base.wire-edm   …
                       │
Fanuc-style base  fanuc.mill        control series   haas   (M97 local subprograms and other codes every Haas control shares)
                       │                              │
                       │            control model    haas.ngc     haas.classic     haas.1995
                       │                              │
machine type      haas.ngc.mill  extends ["fanuc.mill", "haas.ngc"]          haas.ngc.lathe …

machine model     haas.vf3ss     (hardware only; never extends a control)

shop machine      acme.vf3ss-cell1   extends ["haas.ngc.mill", "haas.vf3ss"]   (private, in the shop folder)
```

A file's `extends` is a list. Files are applied in order, then the file itself goes on top. That lets a control share pieces between its mill and lathe files: `haas.ngc.mill` extends `["fanuc.mill", "haas.ngc"]`, where `haas.ngc` (and `haas` above it) holds M97 local subprograms and the other codes both machine types use.

| Layer | Holds | Examples |
|---|---|---|
| `process` | What every machine of this kind has: address meanings (X, Y, Z, U, W, I, J, K…), the core G and M codes, modal groups, the "process on" codes (spindle, beam, torch, wire), which checks apply | `base.common`, `base.mill`, `base.lathe`, `base.mill-turn`, `base.router`, `base.laser`, `base.plasma`, `base.waterjet`, `base.wire-edm`, `base.sinker-edm` |
| `series`, `control`, `type` | The control's language: its codes, cycles and their words, sub calls, foreign codes, number format, the options it knows about | `haas`, `haas.ngc`, `haas.ngc.mill`, `okuma.p200.lathe`, `fanuc.0i.mill`, `linuxcnc.mill` |
| `model` | One machine model's hardware: travel, max RPM, rapids, tool changer, standard options | `haas.vf2`, `haas.vf3ss`, `okuma.lb3000` |
| `machine` | One real machine in one shop: its options, in-house M-codes, custom macros, check settings | `acme.vf2-cell3` |

Model files never extend a control file, so one `haas.vf3ss` serves a VF-3SS with either Haas control. See [Controls, machine models and options](#controls-machine-models-and-options).

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
| Built into the program (compiled into the .exe) | GlossCode releases | Built in, read-only |
| Shop folder (set in Settings, can be a network path) | The shop | With everyone who points at that folder |
| `%APPDATA%\GlossCode\machines` (`~/.config/glosscode/machines` on Linux) | The user | No |

The personal folder is the platform's standard user data folder, not a folder beside the program, so it works when GlossCode is installed in `Program Files` and survives updates. See [Packaging](architecture.md#packaging) for the portable mode that keeps everything beside the .exe instead.

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
  "layer": "machine",                // process | series | control | type | model | machine
  "extends": ["haas.ngc.mill", "haas.vf2"], // a machine type, then a model
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

The **Columns by word type** layout reads its columns from `columns`, in order from left to right. The built-in order puts the most used words first: the G column (motion, canned cycles, cycle cancel and spindle speed mode G96/G97, which rarely share a line with a motion code), X, Y, Z, the arc and cycle words, F, S, the spindle M-codes, the tool, coolant, then other G-codes, other M-codes, rare words and the comment. A word goes in the first column that matches it, so a code listed in an earlier column (M03 under Spindle) never reaches a later catch-all (M).

```jsonc
"columns": [
  { "id": "MOT",  "label": "G",       "groups": ["motion", "canned cycle"], "cycles": true, "codes": ["G96", "G97"],
    "carry": "motion", "aligns": true, "alignsExcept": ["G80"] },
  { "id": "X",    "label": "X",       "letters": ["X", "U"], "carry": "position", "aligns": true },
  { "id": "SPIN", "label": "Spindle", "codes": ["M03", "M04", "M05"], "carry": "spindle", "aligns": true },
  { "id": "G",    "label": "Other G", "letters": ["G"] },
  { "id": "OTHER","label": "Other",   "other": true },
  { "id": "NOTE", "label": "Comment", "comments": true }
],
"programOrderCodes": ["M00", "M01", "M02", "M30", "M06", "M97", "M98", "M99", "G65", "G04", "G10", "G28", "G30", "G53", "G92"]
```

- `letters`: any word starting with these letters. `codes`: these exact codes. `groups`: every code in these modal groups. `cycles`: every canned cycle the machine has. `about`: the column heading's tooltip.
- `carry`: what an empty cell shows in grey when a modal value is still in effect from an earlier line: `motion` (the motion code or active cycle), `position` (where the axis was last sent), `cycle` (R and Q kept by an active canned cycle), `value` (the last F or S), `spindle`, `coolant` and `tool`. Columns with no `carry` stay empty. A carried value stays on one line and is cut short with … if it doesn't fit.
- `aligns`: a line is laid out in columns only when it has a word from one of these columns (motion, axes, arc and cycle words, F, S, spindle). Codes in `alignsExcept` don't count, so the safety line `G80 G20 G90 G17 G40 G49` isn't pulled into the grid by its G80.
- `programOrderCodes`: a line with any of these codes, or with an Okuma keyword (`CALL`, `RTS`, `GOTO`…), is shown in program order even if it has axis words. These are lines that don't make a cutting or positioning move: program stop (M00), optional stop (M01), program end (M02, M30), tool change (M06), subprogram and macro calls and returns (M97, M98, M99, G65), dwell (G04, where X or P is a time), data setting (G10), reference return (G28, G30), machine coordinate moves (G53) and coordinate system setting (G92 on mills, G50 on lathes). Lathes leave out G92, which is a threading cycle there.
- Lines in program order, and lines with no words at all for the aligned columns (the program number, the safety line, a tool preselect, a comment), don't split into columns: their words fill cells from the left in the order they're written.
- Several words in one cell stack one above the other, so a column is only as wide as its widest single word and a line with a stacked cell gets taller.
- Like other arrays, a layer that sets `columns` or `programOrderCodes` replaces the whole list. Only columns used by lines laid out in columns are shown, so extra columns cost nothing.

## Controls, machine models and options

Programming manuals are written for a control, not a machine. The Haas NGC mill manual covers every NGC mill, and none of the Haas manuals reviewed so far mentions the VF-3 or the Super Speed models (see [haas-ngc.md](manual-reviews/haas-ngc.md)). So the definitions are arranged the same way: the control decides the language, and the machine model only changes a handful of things that depend on the hardware.

### The tree

```
Control series     Haas                                    Okuma                 Fanuc
                    │                                       │                     │
Control model      Haas NGC        Haas classic   Haas 1995 OSP-P200L  OSP7000L   0i-MF   0i-TF …
                    │
Machine type       NGC mill        NGC lathe
                    │                │
Machine model      VF-2  VF-3  VF-3SS  UMC-750 …    ST-10  ST-20  ST-30 …
                    │
Options            ☑ Side-mount tool changer  ☐ Gearbox  ☑ Probing  ☑ Macros  ☐ Rotation and scaling  ☐ Through-spindle coolant …

Every level also has   Generic   (only what all the entries under it share)
```

- **Control series** is the builder of the control: Haas, Okuma, Fanuc, Siemens, Mazak, LinuxCNC.
- **Control model** is one generation of that control, written from one set of manuals: Haas NGC, Haas classic, the 1995 Haas control, Okuma OSP-P200L. Manual editions that agree go in one control model. Where an edition changed something, the entry says which (see **Editions** below), so Haas NGC doesn't split into "NGC 2020" and "NGC 2025".
- **Machine type** is mill, lathe, mill-turn and so on, because the same control speaks a different language on each (G50 is the spindle limit on a Haas lathe and scaling off on a Haas mill). A control that only exists for one type, like the OSP-P200L lathe control, has one entry here.
- **Machine model** holds the hardware: travel, max RPM, rapids, tool changer type and pocket count, gearbox, and which options are standard. Models with the same hardware facts can share a group node (VF-2 and VF-3 differ only in travel), and a model only needs its own entry when something about it differs.
- **Options** are checkboxes, not a tree level, because they combine freely. A VF-3SS can have probing, macros and rotation and scaling in any mix, and a tree would need a branch for every mix. The model sets the defaults; the user ticks what their machine really has.

A machine model that was sold with several controls appears under each of them, but it's one file. The VF-3SS file lists the control models it can go with, and the tree shows it under each.

### Generic

Each level has a **Generic** entry: "Haas NGC mill, any model", "Haas NGC, any type", "Haas, any control". It's for a program that has to run on more than one machine, or when you don't know the exact machine yet.

Generic is worked out by GlossCode from the entries under it, never written by hand, so it can't fall out of step:

- A code that every entry has, with the same meaning, is in Generic and reads normally.
- A code that every entry has but with different meanings (G50 across mill and lathe, M39 on umbrella and side-mount changers) is shown as **varies by machine**. Hovering over it lists each meaning, and the check `varies-by-machine` gives an info.
- A code that only some entries have isn't in Generic's autocomplete or Insert menu. If a program uses it, the check `not-on-every-machine` names the entries that lack it ("M41: not on machines without a gearbox"). This turns Generic into a portability check.
- Options work the same way: Generic for a machine model assumes only the options that model always has.

The higher the level, the smaller Generic gets. "Haas NGC mill, any model" is nearly the whole NGC mill language, while "Haas, any control" is little more than the shared Fanuc-style core. The picker shows how many codes each Generic has so that's not a surprise.

### What changes between machine models and options

What the Haas manuals say depends on the machine rather than the control. They describe these in general terms ("if your machine has a gearbox", "side-mount changer only", "varies by model") and don't say which model has what. That's what the machine model and options fill in:

| Hardware fact | Changes in GlossCode | Manual reference |
|---|---|---|
| Max spindle speed | S plausibility check, speed clamp hints | Spec sheet, not the manuals |
| Rapid and max feed rate | Feed plausibility, cycle time estimate | Spec sheet; mill 2025 p. 474 says it varies by model |
| Travel | A future out-of-travel check, the Z timeline scale | Spec sheet; VF 1995 p. 2 lists VF-0 to VF-3 |
| Gearbox or not | M41/M42 valid, or flagged as having no effect | Mill 2025 pp. 203, 437; VF 1995 p. 91 (VF-0 has none) |
| Tool changer: umbrella or side-mount | Tool pre-call ("T next after M06") hint, M39, the heavy-tool flags, broken tool detection | Mill 2025 pp. 162–170, 437, 444, 524 |
| Model-only codes | G143 (VR models only), tailstock settings (ST-20, 30 and 40), G199 (dual spindle) | Mill 2025 p. 397; lathe 2020 pp. 142–147, 227 |
| Purchased options (through-spindle coolant, high-speed machining, macros, rotation and scaling, probing) | Codes that need the option are hidden or flagged when it's off | Mill 2025 pp. 242, 353, 438 |

A code or cycle that depends on hardware or an option lists it in `needs`, the way lathe cycles already list `liveTooling`. A code shows up in autocomplete and the Insert menu when everything in its `needs` is on:

```jsonc
// haas.ngc.mill (machine type)
"codes": {
  "M41":  { "long": "GEARLOW",  "short": "GLOW", "desc": "Low gear",  "needs": ["gearbox"] },
  "M42":  { "long": "GEARHIGH", "short": "GHI",  "desc": "High gear", "needs": ["gearbox"] },
  "G143": { "long": "TLEN5AX",  "short": "TL5X", "desc": "Five-axis tool length offset", "needs": ["vr"] },
  "G51":  { "long": "SCALEON",  "short": "SCLON", "desc": "Scaling on", "needs": ["rotationScaling"] }
}
```

(M41, M42 and G143 aren't in the dictionary yet; this shows the shape.) In a program, a code whose `needs` aren't met still reads as English, so old programs make sense, and a new `needs-option` check says why: "This machine has no gearbox, so M41 has no effect."

### Editions

Manuals for one control model don't always agree. The 2020 NGC mill manual says M39 turns the side-mount changer and the 2025 one says the umbrella changer. Any entry can carry its own `source`, and an entry that changed between editions can give both:

```jsonc
"M39": { "long": "TCROTATE", "short": "TCROT", "desc": "Rotate the tool carousel",
         "source": [{ "manual": "Haas Mill 96-8210", "revision": "N (2020)", "page": 417 },
                    { "manual": "Haas Mill 96-8210", "revision": "V (2025)", "page": 444 }],
         "unsettled": "2020 says side-mount changer, 2025 says umbrella" }
```

`unsettled` shows in the word's tooltip until someone checks it on a machine.

### The files

One file per node, each extending its parent, so a level holds only what's different from the one above. The ids follow the tree:

| Level | Holds | Examples |
|---|---|---|
| `series` | What every control from this builder shares | `haas`, `okuma`, `fanuc` |
| `control` | One control generation's language for all machine types | `haas.ngc`, `haas.classic`, `haas.1995`, `okuma.p200` |
| `type` | That control on one machine type: the codes, cycles, checks and options it knows | `haas.ngc.mill`, `haas.ngc.lathe`, `okuma.p200.lathe` |
| `model` | Hardware only: travel, RPM, rapids, tool changer, standard options, and which `type`s it goes with | `haas.vf3ss`, `haas.st20`, `okuma.lb3000` |
| `machine` | One real machine in a shop: the type and model, the options ticked, in-house codes | `acme.vf3ss-cell1` |

```jsonc
// haas.vf3 (machine model)
{ "id": "haas.vf3", "layer": "model", "name": "VF-3", "group": "VF",
  "types": ["haas.ngc.mill", "haas.classic.mill"],
  "machine": { "travel": { "X": 40, "Y": 20, "Z": 25 }, "maxRpm": 8100,
               "rapid": 1000, "maxFeed": 650,
               "toolChanger": "umbrella", "tools": 20 } }

// haas.vf3ss (machine model): the VF-3 with the Super Speed hardware
{ "id": "haas.vf3ss", "layer": "model", "name": "VF-3SS", "extends": ["haas.vf3"],
  "machine": { "maxRpm": 12000, "rapid": 1400, "maxFeed": 833,
               "toolChanger": "side-mount", "tools": 31,
               "options": { "gearbox": false } },
  "optional": {                     // options with values: ticking one applies its machine section
    "spindle15k": { "label": "15,000 rpm spindle", "machine": { "maxRpm": 15000 } }
  } }

// acme.vf3ss-cell1 (shop machine)
{ "id": "acme.vf3ss-cell1", "layer": "machine", "extends": ["haas.ngc.mill", "haas.vf3ss"],
  "machine": { "options": { "probe": true, "macros": true, "tsc": true } } }
```

- `types` lists the controls (as machine types) a model was sold with, newest first. The tree shows the model under each, and the picker selects the first.
- `group` puts models under one node in the tree (VF, UMC, ST). A model can extend another model and only say what's different: the VF-3SS extends the VF-3 and changes the spindle, rapids, feed and tool changer, keeping its travel and group.
- `optional` lists options that change a value, not just turn codes on. Ticking the 15,000 rpm spindle raises `maxRpm`, so the S check follows. Plain on/off options (probing, macros) stay in `options`.
- Facts that don't change the program, like table load and ballscrew pitch, stay out. They can go in `notes`.
- A shop machine extends a machine type, then a model. The model goes last, so its limits and options apply on top of the control.
- Model files never extend a control file, so one `haas.vf3ss` serves both the NGC and classic versions.

Built-in model files follow the manual rule above: numbers come from the builder's spec sheet or the machine's plate, with `source` saying which, and a model ships `verified: false` until someone checks it. The VF-3 and VF-3SS numbers above are from a summary of Haas's published specs, not yet checked against the spec sheets.

### Picking a machine in GlossCode

The machine picker (File › New, the machine chip, and **New machine profile…**) shows the tree as columns, left to right: series, control, type, model. Each column has **Generic** at the top. A search box above finds any entry ("3ss", "p200"), and searching a model that went with several controls shows one result per control ("VF-3SS · Haas NGC", "VF-3SS · Haas classic").

Under the columns are the option checkboxes, ticked from the model's standard options. The profile window then shows the hardware with each value's source ("12000 from VF-3SS"), and the user can change any of it. The saved profile is a small machine file, so a later fix to the built-in VF-3SS file reaches it.

The machine menu shows the whole path: "Haas NGC mill · VF-3SS". Picking a different model or Generic is one change in **Edit this profile…**, and the Problems panel re-checks the open program.

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
3. Add the `machine` section in place of the mockup's `MACHINES` list, and the shop and personal folders. Split today's profiles into a machine type plus a model file (the VF-2 profile becomes `["haas.ngc.mill", "haas.vf2"]`), and replace the machine picker with the control tree, Generic entries and option checkboxes.
4. Add `checks`, `lineRules` and the `process` table.
5. The GUI editor.
6. Thin `verified: false` bases for router, laser, plasma, waterjet and EDM, then brand layers as real programs arrive.

Until step 1 lands, keep editing `data/gcode-words.json` and `data/gcode-rules.json` as today.
