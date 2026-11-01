# Mazak EIA/ISO programming (machining centers, with mill-turn notes)

| | |
|---|---|
| Manuals | Mazak EIA/ISO Programming Manual C735PBT019E, for Mazatrol M1, M2, M32, M Plus and Fusion 640M 5X/Nexus machining centers (2001) · Mazak EIA/ISO Programming Manual for Mazatrol Matrix, INTEGREX IV, H740PB0030E (mill-turn) |
| Copies reviewed | PDFs with text. Pages are chapter-page (C735: 3-10 = PDF 32, 8-1 = PDF 103, 13-1 = PDF 185) |
| Not used | The Mazatrol conversational manual (out of scope) and the 1980s CAM M-2 manual (a poor scan) |
| Reviewed | 2026-11-01. This adds the `mazak` dialect, which inherits `fanuc` and its words |
| Result | Machining-center cycles checked; mill-turn left for later |

## Machining center (C735)

| Mazak | Page | In the dialect |
|---|---|---|
| G75, G78 and G79 are boring cycles, G76 fine boring, G77 back spot facing, G85 reaming. On Fanuc mills G75 and G77–G79 are grinding cycles | 3-10, 13-1 | All are checked cycles in the canned cycle group, with English words |
| Cycles take X Y Z Q R P D K I J E H F L. **L is the repeat count** (L0 stores the data without drilling); D is the depth below R where the rapid ends; I/J slow the last cut; E is a second feed; H picks synchronous tapping | 13-1 to 13-4 | Each cycle lists its words; K is not a repeat count here |
| **F is required on every cycle** | 13-2 | F is in `requires` |
| G04 X in seconds, or P in 0.001 s | 8-1 | Inherits Fanuc's millisecond P check |
| G92.5 rotation, G54.2 dynamic offset, G61.1 shape correction, G40.1–G42.1 shaping, G71.1/G72.1 chamfering-cutter cycles, G02.1/G03.1 spiral, G06.1/G06.2 splines | 3-10 | Not added (decimal codes) |
| Groups otherwise match Fanuc | 3-10 | Inherited |
| M codes (p. 3-11, "machines differ"): M06 tool change, M07 mist, M09 all coolant and air off, M10/M11 tool clamp/unclamp, M19 orient, M48/M49 overrides, M50 air blast, M51 through-spindle coolant, M52 tapping coolant | 3-11 | English words under `mazak.mill` |

## Not handled yet

- **Numbers without a decimal point** are either the least increment ("type I", the Fanuc reading) or whole units ("type II"), set by parameter F91 bit 5 (pp. 5-4, 5-5). GlossCode assumes type I. A machine profile setting would cover type II.
- **Dwell details:** a decimal part on P is ignored (P12.345 = 0.012 s), and in G95 mode G04 counts spindle revolutions (p. 8-2).
- **Subprograms:** M98 P(program) H(N number inside it) L(repeats). Without P it calls an N number in the current program, like Haas M97. M99 P(return N) L(new count). Up to 8 levels (pp. 13-26, 13-27). Only M98 P is checked.
- Corner ",C" and ",R" (p. 13-14), macros G65/G66/G66.1/G67 with #variables (13-9), and macro G codes (up to 10 codes in 01–255).
- The missing-F message appears twice on a cycle line (once for the cycle's words and once for the feed check).

## Mill-turn (Matrix INTEGREX IV), not added

Mill-turn programs use one of two G code series, which renumber the turning cycles:

- **T series**, close to Fanuc lathe system A: G50, G98/G99 feeds, G70–G76, G83–G89 face and side drilling, G90/G92/G94 cycles.
- **M series:** G92, G94/G95, G90/G91, and the turning cycles moved to G270–G276, G283–G289 and G290/G292/G294, plus the machining-center cycles above.
- All turning cycles are in group 09 with the drilling cycles. G96/G97 are group 17. Codes from different groups in one block run in group order (p. 3-13).

A Mazak lathe or mill-turn profile currently gets the Fanuc lathe (system A) tables, which fits the T series. The M series needs its own tables.
