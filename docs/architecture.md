# Architecture

GlossCode targets Windows first. Everything is arranged so that a native Linux build is mostly a recompile, with no Wine involved.

## Layers

```
+--------------------------------------------------------------+
|  UI (HTML/CSS/JS in a native webview)                        |
|  editor grid, toggles, autocomplete, previews, modes panel   |
+------------------------------^-------------------------------+
                               |  JSON messages (webview bind)
+------------------------------v-------------------------------+
|  Core (portable C, standard library only)                    |
|  parser · translator · mode tracker · checks · dictionary    |
+------------------------------^-------------------------------+
                               |  ~8 functions
+------------------------------v-------------------------------+
|  Platform file: platform_win32.c  (later platform_linux.c)   |
+--------------------------------------------------------------+
```

### Core (portable C)

- **Parser**: splits each line into words (`N40`, `M03`, `G97`, `S6000`) and keeps comments whole. Remembers each file's line endings, since G-code is often CRLF.
- **Mode tracker**: walks the program in execution order, following subprogram calls, and records the modal state after every line: motion, units, G90/G91, plane, feed mode and rate, spindle, coolant, tool, offsets and compensation.
- **Translator**: turns each word into its English word using the dictionary and the active modes. Values take a unit prefix (`RPM600` under G97, `SFM600` under G96, `IPM25` under G94 and G20).
- **Checks**: one pass in execution order that carries the machine state and reports problems such as a canned cycle missing a word, a feed move with no F or with the spindle off, a word repeated in a line, or a code that means something else on this control. Rule text and per-control tables come from `gcode-rules.json`; see [checks.md](checks.md).
- **Dictionary loader**: reads `gcode-words.json` and merges machine definition overrides on top.

Rules for the core: C standard library only (`stdio`, `stdlib`, `string`), no `windows.h`, UTF-8 everywhere, `/` in paths, and no compiler-specific extensions.

### Platform file

One file per OS with the same small set of functions:

| Function | Windows | Linux (later) |
|---|---|---|
| List a folder | `FindFirstFileW` | `opendir` |
| Read / write a file | `CreateFileW` | `open` |
| Watch a file for changes | `ReadDirectoryChangesW` | `inotify` |
| Settings folder | `%APPDATA%\GlossCode` | `~/.config/glosscode` |
| Open / save dialog | nativefiledialog-extended (`IFileOpenDialog`) | nativefiledialog-extended (GTK / portal) |
| Create the window | webview (WebView2) | webview (WebKitGTK) |

### UI

The UI is plain HTML, CSS and JavaScript loaded into the webview. It talks to the core through the webview library's bind and eval calls, passing JSON. The clickable mockup in [`mockup/index.html`](mockup/index.html) is the visual reference.

## Build

- MinGW-w64 or clang on Windows, gcc or clang on Linux.
- Avoid .NET, MFC, the registry and MSVC-only features.
- WebView2 ships with Windows 10 and 11, so the .exe stays small.
- Wine is useful during development to test a cross-compiled .exe on Linux. It is not part of what ships.

## Data files

- `data/gcode-words.json`: the base dictionary. Each code has `long` and `short` words and a description. Codes that differ between machine types have `mill` and `lathe` entries.
- `data/gcode-rules.json`: the checks' messages and severities, and per-control tables (Fanuc, Haas, Okuma; mill and lathe) that inherit from each other. The same tables drive the hint text for missing words (`wordHints` plus the cycle word lists), and `cycleHelp` holds the canned cycle guide's text.
- Machine profiles (`machines/<id>.json`): name, control, dialect, mill or lathe, limits, brand codes, and `eob` (true when every line must end with `;`). The parser treats `;` as an end of block on every machine; the writer adds it when the profile says so.
- The mockup embeds both files. After changing either, run `node scripts/sync-mockup-data.mjs`.
- Machine definitions (to come): per-machine JSON that names the machine type and brand and overrides dictionary entries, for example Haas `M97` (local subprogram call) and `M88`/`M89` (through-spindle coolant).
