# Features

This list tracks what the mockup shows and what is still being discussed. Nothing is implemented yet.

## Shown in the mockup

- [ ] Interleaved view: an English line above every G-code line
- [ ] Fixed-width columns (10 characters for long words, 8 for short) so words line up down the whole file
- [ ] Long and short word styles
- [ ] Edit toggle: type G-code or type English
- [ ] Autocomplete for English and G-code words
- [ ] Mode-clash warnings in the status bar
- [ ] Auto N numbering on new lines (step 10, midpoint when there's no room)
- [ ] Renumber that leaves subprogram labels alone
- [ ] Inline expansion of subprogram calls, marked with purple N numbers; edits change the subprogram
- [ ] Show toggle: both lines, English only, or G-code only
- [ ] Active-modes panel for the selected line, with a mark on modes set on that line
- [ ] Line colors: rapid (amber), feed (blue), arc (teal), cycle (magenta)
- [ ] XY toolpath preview that follows subprogram calls
- [ ] Z-by-line timeline in execution order, centered on the selected line
- [ ] Machine selector (mill or lathe, with brand overrides)
- [ ] System Open and Save dialogs

## Under discussion

- [ ] Menu bar (File, Edit, View, Program, Simulate) or a ribbon
- [ ] Undo and redo
- [ ] Find and replace
- [ ] Sort words on each line into a consistent order
- [ ] Column-per-word-type layout (X, Y, Z, F, M and so on each in their own column, with rarely used words at the end)
- [ ] Simulation mode that draws tool moves one at a time
- [ ] Grey out or hide toolpath moves after the selected line
- [ ] A different toolpath color for each tool

## Later

- [ ] Native Linux build
- [ ] Machine definition file format and a few starter machines
- [ ] Logic checks beyond mode clashes
