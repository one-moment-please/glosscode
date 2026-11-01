# Haas VF-Series Operator's Manual, June 1995

| | |
|---|---|
| Manual | Haas VF-Series Operator's Manual, 96-8000, June 1995 (Haas archive copy, 420 pages, scanned) |
| Control | The Haas mill control as of 1995, decades before NGC |
| Reviewed | 2026-11-01, against `data/gcode-rules.json` and `data/gcode-words.json` |
| Result | New dialect `haas1995` (inherits `haas`), two new checks, 25 new dictionary entries |

The manual stays out of the repository. This file records what it says in our own words, with page numbers so anyone with a copy can check it.

## Which parts matter

Page numbers are the manual's own; the PDF page is 20 higher.

| Manual pages | Section | Use for GlossCode |
|---|---|---|
| 45–48 | Alphabetical address codes | **High.** Every letter's meaning, range and what happens with no decimal point |
| 49–50 | G-code summary table | **High.** Every G code with its modal group and default |
| 51–86 | G-code details | **High.** Words each code takes; canned cycles are 66–83 |
| 87–92 | Plane, inch/metric, work offsets, spindle, tool functions | Medium |
| 93–97 | M-code summary and details | **High** |
| 98–104 | Cutter compensation | Medium: entry and exit rules for the comp checks |
| 109–110 | Subroutines (M97, M98, M99) | **High** |
| 111–132 | Macros | Low for now; needed when macro lines get parsed |
| 163–164 | Tapping | Low: rigid tapping notes |
| 1–42 | Programming tutorial | Low: good example programs for test files |
| 133–162 | Quick Code (conversational) | None |
| 165 onward | Operation, installation, electrical, parts, parameters | None, except the spindle and travel specs |

## What differs from today's Haas tables

Today's `haas` dialect describes the current NGC control. The 1995 control is older and differs in these ways:

- **One M-code per line** (pp. 46, 93). NGC allows several. New check `m-per-block`, set by `mPerBlock: 1`.
- **G04 P without a decimal point is milliseconds** (p. 53): `P500` is half a second, `P1.5` is 1.5 seconds. New check `dwell-milliseconds`, set by `dwellNoDecimal: "ms"`.
- **Canned cycle words** (pp. 68–82):
  - G73 and G83 take Q, or I, J and K, as on NGC, but not P. G73 also has a mode where K and Q are both given.
  - G74 and G84 have no J (retract speed). Both start the spindle themselves.
  - G76 and G77 take Q, or I and J, for the shift. These match the NGC table.
  - G87 exists: bore, stop the spindle, and the operator jogs out. NGC dropped it.
  - G88 exists: bore, dwell, stop, jog out. It needs P.
  - All of them take L for repeats, and only work in G17.
- **No G93, G94, G95, G96 or G97** on the mill (pp. 49–50). F is always per minute and S is always RPM. G93, G95 and G96 are flagged as errors; G94 and G97 are flagged as info because they're harmless.
- **Extra work offsets are G110–G129** (p. 85). NGC's G154 P1–P99 is flagged.
- **No M88/M89 through-spindle coolant** in the M-code list (p. 93). They're flagged as a warning, since a machine could have been retrofitted.
- **M16** is a second tool-change code (p. 93).
- **Modal groups** that the Fanuc base doesn't list: G50/G51 scaling, G61/G64 exact stop and G68/G69 rotation.

## What matches

- Distance words with no decimal point read as 0.0001 in or 0.001 mm (pp. 45–48), so `missing-decimal` applies as it does today.
- M97 calls an N label in the same program, M98 calls a separate O program, and M99 returns. An L on the call repeats it (p. 109).
- G28 moves through the given position first, then home, as on Fanuc (p. 56), so `g28-absolute` applies.
- G00 or G01 cancels an active canned cycle (p. 50).
- G98 and G99 choose initial-point or R-plane return (p. 83).

## Not added yet

- G12/G13 circular pockets, G70–G72 bolt patterns, G150 pocketing, G100/G101 mirror, G10 offsets, G65 macros. The manual covers them, but GlossCode has no checks or Insert dialogs for them yet.
- Ranges: D and H are 0–50, T is 1 to the tool count, and N is 0–99999 (pp. 45–47). There's no range check yet.
- Settings that change how cycles behave (Setting 22 peck retract, 27 shift direction, 52 R-plane clearance, 33 Fanuc or Yasnac G52/G92, 58 cutter comp corner style). These belong in the machine definition once the loader exists.
- A 1995 machine profile. Pick "Haas (1995 control)" as the dialect in Machine profiles to use these tables.
