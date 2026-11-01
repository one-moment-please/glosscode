# Siemens SINUMERIK ISO mode (840D sl, 828D, ONE)

| | |
|---|---|
| Manuals | SINUMERIK ONE / 840D sl / 828D ISO Milling Programming Manual, 07/2021 (CNC 4.95/6.15), the main source · SINUMERIK 840D sl/840Di sl/828D/802D sl ISO Turning Programming Manual, 06/2009 · the ISO Dialects function manuals (09/2009, 12/2019, 07/2021), setup only |
| Not used | The 828D NC Programming Manual (Siemens' own language) and Basic Functions manual. Native Siemens programs aren't covered |
| Reviewed | 2026-11-01. This adds the `siemensiso` dialect, which inherits `fanuc` and its words |
| Result | ISO milling added; ISO turning noted |

The ISO mode is a Fanuc-compatible interpreter inside the Siemens control. Its G code groups match the Fanuc 0i MC (appendix B), so most of the `fanuc` tables apply.

## In the dialect

- **Up to 5 M codes per line** (5.5.7): `mPerBlock: 5`.
- **G290 switches to Siemens mode and G291 to ISO mode** (3.1.2–3.1.3). Each goes alone on its line. English words added.
- **M29 puts the spindle in axis mode** for rigid tapping. English word added.
- Fixed cycles use the Fanuc formats with K repeats (6.1), and dwell P is in internal units without a decimal point (5.3.1), as in the `fanuc` tables.

## Not handled yet

- **Numbers without a decimal point:** machine data MD10884 picks "standard" (Fanuc-like, X1000 = 1 mm on IS-B) or "pocket calculator" (X1000 = 1000 mm) (3.1.6). GlossCode assumes standard.
- **Comments:** ";" is a comment as well as ( ), and brackets nest: `N15 (a(b) X100) Y100` runs only Y100 (3.1.7).
- **"/" block skip can appear anywhere** in a line and skips to the end of it; /1–/9 are separate levels (3.1.8).
- **G04 must be alone on its line** (5.3.1). An option (MD20734 bit 2) makes G04 count revolutions in G95.
- Fixed cycles need a tool length offset active first (6.1.1). Not checked.
- **Subprograms:** M98 P packs a count in front of a 4-digit program number (M98 P30021 runs 0021.mpf three times), or M98 P L. Up to 16 levels. M99 P returns to that N. Up to 10 M codes can be mapped to macros (6.2–6.3, 5.5.6). Only the plain M98 P check runs.
- **G54 P1–P48** extended offsets, G54.1, G05.1 (calls CYCLE305), G72.1/G72.2 contour repetition.
- No ISO and Siemens commands mixed in one block.
- **Manual error:** the printed formats for G73, G76 and G82 leave out Z.

## ISO turning (PGT 06/2009), not added

The lathe G code system (A, B or C) is set by MD10881, and **the default is system B** (PGT 1.1.6), where Fanuc lathes are usually A. The cycles use the Fanuc formats: G71 U R / P Q U W, G72, G73, G70, G74/G75 X Z P Q R F, G76. A Siemens ISO lathe profile gets the Fanuc system A tables for now, so on a system B machine G90/G91 and G94/G95 would read wrongly.
