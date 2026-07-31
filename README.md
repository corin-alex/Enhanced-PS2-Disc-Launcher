# Enhanced PS2 Disc Launcher

A disc launcher for PlayStation 2 consoles patched with [MechaPwn](https://github.com/MatheusBond/MechaPwn), aimed at models where MechaPwn does not fully disable region locking — notably Japanese consoles such as the SCPH-50000.

On those consoles, import discs cannot be started from the console menu. This launcher boots the disc itself, bypassing the PlayStation 2 logo check, and can override the OSD language so import games do not fall back to English.

## Features

- Skips the PlayStation 2 logo check, so import and master discs boot correctly.
- Optional OSD language override for import games (`disc-launcher.cnf`).
- Optional confirmation prompt instead of launching the disc immediately.
- Matches the PlayStation driver's video mode to PS1 import discs, via [PS1VModeNeg](https://github.com/ps2homebrew/PS1VModeNeg).

## Installation

`disc-launcher.elf` runs from a PS2 memory card, USB drive, or HDD. Grab it from the [Releases](../../releases) page or [build it yourself](#building-from-source).

Recommended setup with Free MCBoot:

1. Copy `disc-launcher.elf` into the `APPS` folder of your Free MCBoot memory card.
2. For PS1 video mode switching, put `PS1VModeNeg.elf` in the same folder.
3. To override the console language, copy `disc-launcher.cnf` there too and edit it — see [Configuration](#configuration).
4. In `Free MCBoot Configurator`, open `Configure OSDSYS options...`.
5. Enable `Skip Disc Boot` so the console stops auto-booting discs on insert.
6. Point the `Launch disc` item at `mc?:/APPS/disc-launcher.elf`.
7. Go back, save the CNF to `MC0` or `MC1` (wherever your FMCB card is), and exit.

To play a game: insert the disc, then pick `Launch disc` from the Free MCBoot menu (either order works).

## Configuration

Create `disc-launcher.cnf` next to `disc-launcher.elf`. Both settings are optional.

```ini
# Language override
# 0=Japanese, 1=English, 2=French, 3=Spanish
# 4=German, 5=Italian, 6=Dutch, 7=Portuguese
language = 1

# Auto launch disc (0=no, 1=yes)
autolaunch = 1
```

| Setting | Values | Default | Notes |
| --- | --- | --- | --- |
| `language` | `0`–`7` | console setting | Only applied when the disc's region is recognised **and** differs from the console's. Invalid or missing values fall back to the console setting. |
| `autolaunch` | `0`, `1` | `1` | With `0`, the launcher shows the detected game and waits for **X**. |

## PlayStation 1 discs

The launcher compares the region of the PS1 disc with the console's. On a mismatch (for example a PAL game in an NTSC console) it runs `PS1VModeNeg.elf` to adjust the PlayStation driver's video mode; otherwise the game boots normally. `PS1VModeNeg.elf` must sit in the same location as `disc-launcher.elf` (memory card, USB, or HDD) — if it is not found, PS1 games are launched normally. Download it [here](https://github.com/ps2homebrew/PS1VModeNeg).

## Building from source

No local PS2 toolchain required — the build runs in the [ps2dev](https://github.com/ps2dev/ps2dev) container, which is published for both x86 and ARM (so it is native on Apple Silicon):

```sh
./build.sh                      # macOS, Linux, WSL, Git Bash
docker compose run --rm dev     # same thing, any platform
```

The build produces `dist/disc-launcher.elf` alongside a copy of `disc-launcher.cnf`, so the whole `dist/` folder can be copied straight to a memory card. Intermediate files stay in `out/`, and sources live in `src/`. `./build.sh clean` removes both output folders; `./build.sh shell` opens a shell inside the toolchain. If you already have a ps2dev toolchain installed (`$PS2SDK` set), `./build.sh` and plain `make` use it directly.

VS Code users can instead run `Dev Containers: Reopen in Container` for a ready-made environment. See [CLAUDE.md](CLAUDE.md) for the project layout and internals.

## Credits

Based on [Retro GEM Disc Launcher](https://github.com/CosmicScale/Retro-GEM-PS2-Disc-Launcher) by [CosmicScale](https://github.com/CosmicScale).
Updated by [Exalandru](https://github.com/corin-alex).
