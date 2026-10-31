# GlossCode

GlossCode is a G-code editor that shows a plain-English line above every line of code, with each code lined up under its English word. It tracks active modes, colors rapid and feed moves, and expands subprogram calls inline.

> **Status: design stage.** Nothing is built yet. The screenshots below come from the clickable mockup in [`docs/mockup/`](docs/mockup/index.html), which predates the GlossCode name and still says "G-Code Reader" in its title bar.

![GlossCode editor mockup](docs/images/editor-overview.png)

## What it looks like

Every G-code word sits in a fixed-width column, and its English word sits directly above it:

```
SEQ40      SPINFWD    MODERPM    RPM6000
N40        M03        G97        S6000

SEQ50      TLENON     LOFS01     Z1.0       COOLON
N50        G43        H01        Z1.0       M08
```

The English words come from a dictionary of consistent abbreviations ([`data/gcode-words.json`](data/gcode-words.json)). Each word has a long style (`SPINFWD`, 10 columns wide) and a short style (`SFWD`, 8 columns wide). Values carry their unit as a prefix that depends on the active modes, so `S600` reads `RPM600` under G97 and `SFM600` under G96.

| Long words | Short words | English only |
|---|---|---|
| ![Long words](docs/images/editor-overview.png) | ![Short words](docs/images/short-words.png) | ![English only](docs/images/english-only.png) |

## Planned features

**Editing**
- Edit either line. A toggle switches between typing G-code and typing English, with autocomplete from the dictionary (type `cool` to get COOLON, COOLOFF, and TSCON on a Haas).
- Automatic N numbers on new lines (step 10, or the midpoint when there's no room), plus a Renumber command that leaves subprogram labels alone.
- Warnings when a word clashes with the active mode, such as an inches-per-rev feed while G94 is on.
- Opens and saves through the standard system file dialog, and keeps each file's original line endings.

**Reading**
- Show both lines, English only, or G-code only.
- Subprogram and canned-cycle calls (for example Haas `M97 P1000`) expand inline below the call, marked with purple N numbers. Editing an expanded row edits the subprogram itself.
- An active-modes panel for the selected line: motion, units, absolute or incremental, plane, feed mode and rate, spindle, coolant, tool and offsets. A dot marks modes set on that line.
- Line colors by motion: amber for rapid, blue for feed, teal for arcs, magenta for drilling cycles.

**Previews**
- XY toolpath view that follows subprogram calls.
- A "Z by line" timeline in execution order that keeps the selected line centered.

**Machines**
- Mill and lathe modes, since some codes mean different things on each (G50, G76, G98, G99).
- Per-machine definition files that override dictionary entries (for example Haas M88/M89 and M97).

See [`docs/features.md`](docs/features.md) for the full list and what is still under discussion.

## Platform plan

Windows first, with a native Linux build later rather than shipping Wine.

- **Core in portable C**: parser, translator, mode tracking, checks and dictionary loading. Only the C standard library, no `windows.h`, UTF-8 throughout.
- **One small platform file per OS**: `platform_win32.c` now, `platform_linux.c` later, each with the same handful of functions (read and write files, watch for changes, find the settings folder, open the window).
- **UI in a native webview** through the [webview](https://github.com/webview/webview) library: WebView2 on Windows 10/11, WebKitGTK on Linux. The same HTML/JS UI runs on both.
- **System file dialogs** through [nativefiledialog-extended](https://github.com/btzy/nativefiledialog-extended).
- **Build with MinGW-w64 or clang** so the same toolchain works on both systems.

More detail is in [`docs/architecture.md`](docs/architecture.md).

![Open uses the standard Windows dialog](docs/images/native-open-dialog.png)

## Repository layout

```
data/gcode-words.json   Starter abbreviation dictionary (long and short styles, mill/lathe splits, Haas overrides)
docs/architecture.md    Core, platform layer, UI and build plan
docs/features.md        Feature list and open questions
docs/images/            Mockup screenshots
docs/mockup/index.html  Clickable mockup; open it in a browser
examples/               Sample programs used by the mockup (Haas mill and lathe)
```

## Try the mockup

Open [`docs/mockup/index.html`](docs/mockup/index.html) in any browser. Click a word to edit it, switch the Edit, Words and Show toggles, and use **▸ show N1000** to expand the subprogram. It loads its fonts from Google Fonts, so it looks best online.

## License

Not chosen yet.
