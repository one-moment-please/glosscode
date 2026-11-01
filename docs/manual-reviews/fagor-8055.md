# Fagor 8055 and 8055i (mill and lathe)

| | |
|---|---|
| Manuals | 8055/8055i M Programming Manual Ref. 1501, Soft V01.6x (main mill source) · 8055 Programming Manual Ref. 1402 and 8055 M Ref. 1110 (earlier mill editions) · 8055 ·T· Programming Manual Ref. 1110, Soft V01.3x (lathe) · a Portuguese 8055/8055i M Ref. 0706 was not used |
| Copy reviewed | PDFs with text. Page numbers are the manuals' printed "·n·" footers |
| Reviewed | 2026-11-01. No dialect yet |
| Result | Fagor looks like Fanuc but renumbers many G codes, uses different cycle words and has its own block format. It needs its own dialect, and the parser needs work first |

Fagor was last in the suggested order. This review records what a `fagor` dialect would need, so it can be built when a Fagor machine is a target.

## Block format (mill pp. 33–36)

A line is: optional block skip (`/`, `/1`, `/2`, `/3`), an optional N label of up to 8 digits, ISO words or a high-level statement, an optional **N repeat count of up to 4 digits at the end of the line**, and an optional `;` comment.

- **N means two things.** At the start of a line it's a label; at the end it's how many times to repeat the line. N0 at the end makes the move without running the active cycle.
- **Comments start with `;`.** Parentheses don't hold comments. They hold high-level statements, such as `(SUB 12)`, `(RET)`, `(CALL 3)` and `(IF …)`.
- Empty lines aren't allowed, and a program starts with a `%name,attributes,` line.

Today's parser would read every parenthesised statement as a comment and the end-of-line N as a second label, so the block format comes first.

## Mill G codes that differ from Fanuc (pp. 62–64)

The modal and default columns of these tables need checking against the page images.

- **Units:** G70 inch, G71 metric. G20/G21 are work zone limits and G22 turns zones on and off.
- **Tool length:** G43 on, **G44 off**. G49 is the inclined plane, not a cancel.
- **Work offsets:** G54–G57 absolute, **G58/G59 additive**, G159 extended.
- **Mirror image:** G10–G14. G15 sets the longitudinal axis, G16 the plane from two addresses.
- **Corners and arcs:** G05 round corner, G07 square corner, G06 arc center in absolute, G08 tangent arc, G09 three-point arc, G36 rounding, G37/G38 tangential entry and exit, G39 chamfer.
- **G28/G29** select the spindle or swap axes, G30 synchronizes spindles. **G32** is inverse time feed.
- G47 tool coordinates, G48 TCP, G50 rounding, G51 look-ahead, **G52 move to contact**.
- **Patterns and pockets:** G60–G65 multiple machining patterns, G66–G68 irregular pocket, G69 variable peck drilling, G87 rectangular pocket, G88 circular pocket.
- G72 scaling, G73 rotation, **G74 home search**, G75/G76 probing, G77/G78 axis slaving, G79 changes cycle parameters.
- **Cycles:** G81 drill, G82 drill with dwell, G83 deep hole, G84 tap, **G85 ream**, G86 bore, G89 bore with dwell. G210 bore milling, G211/G212 thread milling.
- **G92** presets coordinates or limits spindle speed, **G93** sets the polar origin, G94/G95 feed modes, **G97** constant tool center speed, G98/G99 return level.

## Mill canned cycles (pp. 144–145, 158)

- The words aren't Fanuc's: `G81 G98/G99 X Y Z I K`, where **Z is the reference plane, I the depth and K the dwell in hundredths of a second**. There's no R.
- A cycle can't be defined while G02, G03, G08, G09, G33 or G34 is active.
- A defined cycle runs after every move, and the N count at the end of a line repeats it.

## Dwell (pp. 107–108)

- **G04 K, in hundredths of a second** (K50 is 0.5 s).
- G04 K0 stops block preparation instead of dwelling.

## Lathe (8055 ·T·)

The lathe numbers its cycles differently again:

- G60/G61 face and side drilling and tapping, G62/G63 slot milling.
- G66 pattern repeat, G68 roughing along X, G69 roughing along Z.
- G81/G82 straight turning and facing, G83 drilling, G84/G85 curved turning and facing, G86/G87 threading, G88/G89 grooving.
- G151/G152 X as diameter or radius, G233 thread withdrawal on interrupt, G15 the C axis.

## Subroutines (p. 391)

- A subroutine is `(SUB n)` … `(RET)`. It's called with `(CALL n)`, `(PCALL n, …)` with local parameters, or `(MCALL n)` modal.
- Numbers 0–9999 are general and 10000–20000 belong to the machine builder. A number must be unique across the whole control memory, not just the program.
- Only subroutines in RAM run; others need EXEC.
- There's no M98/M99 in the core language.

## M codes (chapter 5, about pp. 66–80)

Apart from the usual M00–M06, M08/M09 and M30, Fagor leaves M codes to the machine builder. They weren't extracted.

## What a `fagor` dialect would need

1. Parser: `;` comments, parenthesised statements kept as statements, N at the end of a line read as a repeat count.
2. Its own G tables (not inheriting `fanuc`): G70/G71 as units (`unitCodes`), G44 as tool length off, the renumbered cycles, and every Fanuc meaning above in `foreign`.
3. Cycle words Z/I/K with no R, and `dwellWords` K in hundredths.
4. `subCalls` for CALL/PCALL/MCALL and `subReturn` RET, which needs statement parsing.
