# Checks

The Problems panel lists every mistake GlossCode finds in a program, with the worst ones first on each line. Click a problem (or press F8) to jump to its line. Each problem also shows as a red, amber or blue mark in the margin.

The checks are split in two:

- **Data** in [`data/gcode-rules.json`](../data/gcode-rules.json): the message, severity and explanation for every rule, plus the per-control tables the rules read (modal groups, canned cycle words, codes that mean something else on that control, number formats, limits).
- **Logic** in the core (`checks.c`, still to be written; the mockup's `checkProgram()` is the reference). It makes one pass over the program in the order the machine runs it, following subprogram calls, and carries the machine state as it goes: units, absolute or incremental, feed mode and rate, spindle, tool, length offset, cutter comp, the active canned cycle and the tool position.

Adding a control or fixing a cycle's word list is a data change. Adding a new kind of check is a code change plus a new entry in `rules`.

## Severities

| Severity | Meaning |
|---|---|
| error | The control will alarm, or the program will cut something different from what it says. |
| warning | Very likely a mistake, but some shops do it on purpose. |
| info | Worth a look. Often fine. |

A machine definition file will be able to change any rule's severity or turn it off.

## Rules

| Rule id | Severity | What it catches |
|---|---|---|
| `dup-address` | error | The same letter twice in one line, like `Y2. Y3.` (G and M are exempt) |
| `empty-value` | error | A letter with no number, like `X` on its own |
| `modal-group-clash` | error | Two codes from one group on one line, like `G90 G91` or `G00 G01` |
| `m-conflict` | error | M-codes that contradict each other, like `M03 M05` or `M08 M09` |
| `foreign-code` | warning or error | A code that means something else on this control, like `G54` on an Okuma (X-axis length offset) or `M97` on a Fanuc |
| `cycle-missing-word` | error | A canned cycle without a word it needs, like G81 with no R |
| `cycle-missing-choice` | error | A cycle that needs one of several words, like Haas G83 with no Q and no I/J/K |
| `cycle-stray-word` | info | A word the cycle doesn't use, like Q on a G81 |
| `missing-label` | error | M97 P, or a lathe cycle's P/Q, pointing to an N number that isn't in the file |
| `no-feed-rate` | error | A G01/G02/G03 move or feed cycle before any F |
| `feed-mode-changed` | warning | F set under G94 and then used under G95 (or the reverse), which turns 10 ipm into 10 ipr |
| `feed-plausibility` | info | A per-rev feed that's large (over 0.05 in or 1.5 mm) or a per-minute feed that's tiny |
| `spindle-off-feed` | warning | A cutting move with the spindle stopped or never started (Haas taps start the spindle themselves, so they're exempt) |
| `no-speed` | warning | M03/M04, or a tapping cycle, with no S set |
| `missing-decimal` | warning | A distance word with no decimal point. `X1` is 0.0001 in on a Fanuc-style control |
| `arc-no-center` | error | G02/G03 with no R and no I/J/K |
| `arc-r-and-ijk` | warning | An arc with both R and I/J/K |
| `arc-full-circle-r` | error | A full circle written with R |
| `arc-radius-mismatch` | warning | The arc's end point isn't the same distance from the center as its start |
| `comp-start-on-arc` | error | G41/G42 turned on during an arc |
| `comp-no-d` | warning | G41/G42 with no D offset given |
| `tlo-missing` | warning | A Z move after a tool change before G43 H (G56 H on Okuma) |
| `tlo-h-mismatch` | info | `T3` with `H04` |
| `cycle-left-on` | warning | A canned cycle still active at a tool change or the program end |
| `comp-left-on` | warning | Cutter comp still on at a tool change or the program end |
| `g28-absolute` | warning | `G28 Z0.` under G90, which moves through work Z0 on the way home |
| `css-no-clamp` | warning | G96 on a lathe before any `G50 S` max RPM |
| `dwell-no-time` | error | G04 with no time |
| `rapid-below-zero` | info | A G00 to a negative Z (assuming Z0 is the top of the part) |
| `no-program-end` | warning | No M30 or M02 |
| `unknown-word` | info | A code that isn't in the dictionary or this machine's file |

`CHECKS_DEMO.nc` and `LATHE_CHECKS.nc` in the mockup have one of these mistakes on almost every line.

## Numbers and units in the English line

- The English line always shows a decimal point: `X1.` reads `X1.0`, and `X.25` reads `X0.25`.
- A distance word (X, Y, Z, U, W, I, J, K, R, Q) written without a decimal point is shown the way the control reads it. On Fanuc and Haas that's the smallest increment (Okuma is assumed to match until checked), so `X1` reads `X0.0001` in orange under G20, or `X0.001` under G21, and `missing-decimal` flags it. The per-control setting is `numbers` in the rules file, so a control set up to read whole numbers as units can turn this off.
- F and S keep their letter, followed by a small unit: `F12.0 ipm`, `F0.008 ipr`, `F300.0 mmpm`, `S2500 rpm`, `S400 sfm`, `S120 smm`. The unit follows the active G94/G95 (G98/G99 on Fanuc-style lathes), G96/G97 and G20/G21.

## How the rules file is laid out

```jsonc
{
  "rules": {
    "cycle-missing-word": { "severity": "error", "title": "...", "message": "{cycle} ({name}) needs {word}: {meaning}." }
  },
  "dialects": {
    "fanuc": { "mill": { "modalGroups": {...}, "cycles": {...}, "foreign": {...} }, "lathe": {...} },
    "haas":  { "inherits": "fanuc", "mill": { "cycles": { "G83": {...}, "G87": null } } },
    "okuma": { "inherits": "fanuc", "verified": false, ... }
  }
}
```

A dialect inherits from another and overrides by key. Objects like `cycles` and `foreign` merge one entry at a time, and `null` removes an inherited entry (Haas has no G87 back bore; it uses G77). Each cycle entry lists `requires`, `optional`, `choices` (one of several sets), `labels` (words that name N numbers), and flags: `feed` (uses F as a feed rate), `leadF` (F is the thread lead), `speed` (needs S), `spindle` (needs the spindle running), `startsSpindle` (the cycle starts it) and `modal` (false for one-shot lathe cycles).

`verified: false` marks tables written from general knowledge that still need checking against the control's programming manual. The mockup shows a banner in the Problems panel for those.

## How much Haas and Okuma support there is today

| Control | Translation | Checks | Confidence |
|---|---|---|---|
| Haas mill (NGC) | Good for 2.5-axis work | All rules above, with Haas cycle words (I/J/K peck, J retract on taps, L repeats), M97 local subprograms | High |
| Haas lathe (NGC) | Good for turning, facing, grooving and threading | All rules above, with Haas one-block G71/G72/G73/G76, G90/G92/G94 cycles, G50 S clamp | High |
| Fanuc (0i/30i) | Same base as Haas | Same, with Fanuc cycle words and M97 flagged as Haas-only | High |
| Okuma mill (OSP) | G15 H work offsets, G56 H length offset, G53 offset cancel, CALL / RTS subprograms, M02 | All the generic rules, plus Okuma's traps: G54–G59 (length offsets on Okuma), G43, G28, M97, M98, M99 | Medium: first draft, not yet checked against an OSP manual |
| Okuma lathe (OSP) | G90/G91, G94/G95 feed modes, LAP and threading codes named | Generic rules, plus G98/G99, G28, M97, M98 flagged | Low: LAP and threading cycles aren't checked yet |

### Not covered yet

**Haas mill:** G12/G13 circular pockets, G150 pocketing, G70/G71/G72 bolt patterns, G47 engraving, G51/G68 scaling and rotation, 4th and 5th axis (A/B words, G93 inverse time, G234 TCPC, G254 DWO), and macros (`#` variables, IF, WHILE, GOTO, G65 calls). Macro lines are left alone rather than flagged.

**Haas lathe:** live tooling (C axis, G112 polar interpolation, Y axis), sub-spindle and tailstock codes, bar feeder, and tool nose comp checks.

**Okuma, both:** the parts of OSP that aren't Fanuc-shaped. These need their own parser support, not just table entries:

- Sequence names made of letters (`NAT1`, `NLAP1`) and program names like `OSUB`. The mockup only handles numeric `N` and `O` labels.
- User Task variables (`VC1`, `V1`, named variables) and `IF [..] N..` / `GOTO` branches.
- Schedule programs and file types (`.MIN`, `.SSB`, `.SDF`).
- Okuma lathe LAP: the G85/G86/G87/G88 roughing and finishing cycles read a contour defined between G81/G82 and G80, named by a sequence name. Checking these needs the contour block support above.
- Okuma lathe compound threading (G71/G72) and 6-digit tool calls (`T010101`).
- Twin-turret G13/G14 and the Okuma M-codes that differ from Fanuc.

To take Okuma past draft quality, the next step is a handful of real programs from each Okuma machine, plus the control model (P300, P500…) so the tables can be checked against its manual.
