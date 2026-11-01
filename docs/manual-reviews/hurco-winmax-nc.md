# Hurco WinMax Mill NC programming (ISNC)

| | |
|---|---|
| Manuals | Hurco WinMax Mill NC Programming 704-0116-310 Rev B (Feb. 2010, WinMax v8), all NC · WinMax Mill Programming 704-0116-501 Rev D (Jun. 2015, v9.1/10.1), chapter 3 NC Programming only |
| Copies reviewed | PDFs with text. Pages are chapter-page |
| Not used | The conversational chapters (out of scope). The WinMax lathe NC manual isn't available; the lathe conversational, options and getting-started manuals only have settings |
| Reviewed | 2026-11-01. This adds the `hurco` dialect for ISNC (industry standard NC), which inherits `fanuc` and its words |
| Result | ISNC mill tables added. Hurco BNC (Basic NC) is not |

The 2015 manual has the same G code list as 2010 and adds M16/M17, M42, M51, M70, M90/M91, M142–M159, M170–M177 and M210–M214.

## Two NC dialects on one control

WinMax runs **ISNC** (Fanuc-like) or **Hurco BNC** (p. 1-9). Some codes mean different things:

| | ISNC | BNC |
|---|---|---|
| Units | G20/G21 | G70/G71 |
| G74 / G75 | Left-hand tap / – | Single / multi-quadrant arcs |
| G86 | Bore, rapid out | Bore with spindle orient |
| G87 | Back bore | Chip breaker |
| G88 | Bore, dwell, manual out | Rigid tap |
| Cycle R and Z | R is an absolute level and required | R is a positive distance from the initial point (G99 only); Z is always positive and incremental |

GlossCode's `hurco` dialect is ISNC. BNC would need its own dialect.

## What the dialect does

| Hurco ISNC | Page | In the dialect |
|---|---|---|
| **Numbers mean what they say** (X1 is one unit). Whole numbers are scaled only in F (BNC), R rotation, P/X dwell and P scaling | 1-10 | `noDecimal: "asWritten"` |
| Dwell G04 P or X in seconds, 0.001–9999.999. A whole-number P is scaled by 0.001 (or 0.0001, by parameter), so P5 is 0.005 s | 2-20 | Inherits Fanuc's millisecond P check |
| **No G95, G96 or G97**; feed mode is G93/G94 (G94.1 too) | 2-4 to 2-7 | G95 and G96 are errors, G97 an info note |
| Groups otherwise match Fanuc: scaling G50/G51, macros G65–G67, G61/G64, rotation G68/G69, polar G15/G16 | 2-4 to 2-7 | Inherited |
| Cycles take **K or L as the repeat count, 1–6** (K0/L0 stores the data only), P dwell, Q peck or shift, I/J shift | 2-99 | L added to every cycle |
| Several M codes per line; M00/M02 should be alone or last | 5-2 | No limit set |
| M05 also turns the coolant off; M07 mist, M08 flood, M09 both off, M10 both on; M12/M13 rotary clamp; M21 lube; M25 Z to tool-change height; M29 rigid tap | 5-2 to 5-4 | English words under `hurco.mill` |

## Not handled yet

- **The repeat range 1–6** isn't checked.
- **Dwell is modal** and carries into the cycles that follow (p. 2-20).
- **M98 P** packs a repeat count (up to 4 digits) in front of a 4-digit program number: M98 P60050 runs program 50 six times (p. 5-12). The O block sits in the same file after M02. Only the plain M98 P check runs.
- **User G and M codes** (G01–G255, M01–M255) can be mapped to subprograms (p. 1-20). Macros are in chapter 6 (option).
- The first Z after a tool change has to be absolute (p. 5-7). Not checked.
- Decimal codes: G02.4/G03.4, G05.1–G05.3, G07.2/G07.3, G08.1/G08.2, G41.2, G43.4, G50.1/G51.1, G54.1, G68.2/G68.3, G94.1.
- The group defaults are shaded in the table on PDF pp. 48–51 and should be checked against the image.
