# Features

This list tracks what the mockup shows. Nothing is implemented yet.

## Shown in the mockup

**Editor**
- [ ] Interleaved view: an English line above every G-code line
- [ ] Fixed-width columns (10 characters for long words, 8 for short) so words line up down the whole file
- [ ] Long and short word styles
- [ ] Edit toggle: type G-code or type English
- [ ] Autocomplete for English and G-code words
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

**Previews**
- [ ] XY toolpath preview that follows subprogram calls
- [ ] Toolpath colors by move type or by tool
- [ ] Moves after the selected line greyed out, hidden, or shown normally
- [ ] Simulation playback: restart, step back, play/pause, step forward, speed
- [ ] Z-by-line timeline in execution order, centered on the selected line

**Files and machines**
- [ ] Machine selector (mill or lathe, with brand overrides)
- [ ] System Open and Save dialogs

## Later

- [ ] Native Linux build
- [ ] Machine definition file format and a few starter machines
- [ ] Logic checks beyond mode clashes
