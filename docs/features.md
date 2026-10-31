# Features

This list tracks what the mockup shows. Nothing is implemented yet.

## Shown in the mockup

**Editor**
- [ ] Interleaved view: an English line above every G-code line
- [ ] Fixed-width columns (10 characters for long words, 8 for short) so words line up down the whole file
- [ ] Long and short word styles
- [ ] Edit toggle: type G-code or type English
- [ ] Autocomplete for English and G-code words
- [ ] Keyboard editing by word: ← → move between words, ↑ ↓ between lines, Enter edits and Enter again finishes (the word stays selected), Delete removes the word, typing replaces it
- [ ] While editing, Space starts a new word to the right; in G-code, typing a letter after a number does too (`G01X` becomes `G01` and `X`)
- [ ] Shift+Enter breaks the line after the selected word, moving the rest to a new numbered line
- [ ] Caps Lock switches between editing the G-code line and the English line (View › Caps Lock switches between them)
- [ ] Hint text after codes for the words they still need (`G02` shows `R## or I## J##`, `G83` shows `Z## R## Q##`). Clicking R## types the R and hides I and J; clicking I## hides R but keeps J
- [ ] Mode-clash warnings in the status bar
- [ ] Auto N numbering on new lines (step 10, midpoint when there's no room)
- [ ] Renumber that leaves subprogram labels alone
- [ ] Undo and redo
- [ ] Find, matching both G-code and English
- [ ] Sort the words on each line into a consistent order
- [ ] Menu bar: File, Edit, View, Program, Simulate

**Reading**
- [ ] Inline expansion of subprogram calls, marked with purple N numbers; edits change the subprogram
- [ ] Show toggle: both lines, English only, or G-code only
- [ ] Layout toggle: words in program order, or columns by word type with rare words at the end
- [ ] Active-modes panel for the selected line, with a mark on modes set on that line
- [ ] Line colors: rapid (amber), feed (blue), arc (teal), cycle (magenta)
- [ ] Canned cycle guide (F1, Help menu, or "How G76 works…" in the side panel): a sketch of how the cycle moves, an example line with every word it takes (English above), and what each word does and whether it's required. A button starts a new line with that cycle

**Previews**
- [ ] XY toolpath preview that follows subprogram calls
- [ ] Toolpath colors by move type or by tool
- [ ] Moves after the selected line greyed out, hidden, or shown normally
- [ ] Simulation playback: restart, step back, play/pause, step forward, speed
- [ ] Z-by-line timeline in execution order, centered on the selected line

**Files and machines**
- [ ] Machine selector (mill or lathe, with brand overrides): Haas, Fanuc and Okuma
- [ ] End of block: `;` at the end of every line for controls that need it, set per machine profile. Shown after an empty word space in the standard view and written on save. Files that already use `;` are read correctly on any machine
- [ ] English numbers always show a decimal point; distance words without one show what the control reads (X1 → X0.0001)
- [ ] F and S keep their letter with a small unit (ipm, ipr, mmpm, mmpr, rpm, sfm, smm)

**Checks** (see [checks.md](checks.md))
- [ ] Problems panel with severity filters, margin marks, click or F8 to jump to the line
- [ ] Canned cycles missing required words, per control (Haas, Fanuc, Okuma)
- [ ] Feed move with no F, F left over from the other feed mode, cutting with the spindle off
- [ ] Word repeated in a line, two codes from one modal group, contradicting M-codes
- [ ] Missing decimal point, arc geometry, cutter comp start, tool length offset after a tool change
- [ ] Codes that mean something else on the selected control
- [ ] System Open and Save dialogs

## Later

- [ ] Native Linux build
- [ ] Machine definition file format and a few starter machines
- [ ] Okuma checked against an OSP manual and real programs; LAP cycles, sequence names, User Task variables
- [ ] Macro programs (#vars, IF, WHILE) and 4th/5th axis checks
- [ ] Machine limits (travel, max RPM and feed) and part/fixture models for rapid-into-part checks
