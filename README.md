# Enhanced PS2 Disc Launcher

A disc launcher for PlayStation 2 consoles patched with [MechaPwn](https://github.com/MatheusBond/MechaPwn), particularly aimed at Japanese models such as the SCPH-50000 where MechaPwn does not fully disable region locking.
  
On these models, import game discs cannot be launched directly from the console menu. This launcher bypasses the PS2 logo check, allowing import and master discs to boot correctly.
  
It also allows overriding the OSD language when playing import games, which is useful when the console's language settings do not include the disc's region language (e.g., a Japanese console playing a PAL game).
  
As a fork of Retro GEM Disc Launcher, it also sets the Retro GEM Game ID when a [Retro GEM](https://www.pixelfx.co) module is installed, allowing per-game settings for all your physical games.

## Features

### For PlayStation 2 game discs
- Skips the PlayStation 2 logo check, allowing MechaPwn users to launch imports and master discs.
- Optional OSD language override via `disc-launcher.cnf`.
- Sets Retro GEM Game ID.
- Adjusts the video mode of the console's PlayStation driver to match that of the inserted disc if necessary.

### For PlayStation game discs
- Sets Retro GEM Game ID.
- Adjusts the video mode of the console's PlayStation driver to match that of the inserted disc if necessary.

## Instructions

`disc-launcher.elf` can be run from a Memory Card (PS2), USB, or HDD. For a seamless experience, insert the disc before launching the application.

### Recommended setup for Free MCBoot users:
- Put `disc-launcher.elf` in the `APPS` folder on your Free MCBoot memory card.
- To enable video mode switching for imported PlayStation games, place `PS1VModeNeg.elf` in the same location.
- To override console language, create `disc-launcher.cnf` or copy and edit the provided one. see [Language Settings](#Language-Settings)
- Go to `Free MCBoot Configurator` and select `Configure OSDSYS options...`
- Turn on `Skip Disc Boot` to prevent the console from auto-booting game discs when they are inserted.
- Configure the item `Launch disc` so that it points to `mc?:/APPS/disc-launcher.elf`.
- Return to the previous screen. Save CNF to `MC0` or `MC1` (depending on where your Free MCBoot memory card is located) and exit.
- To launch a game, simply insert the game disc, then select `Launch disc` from the Free MCBoot menu. Alternatively, select `Launch disc` and then insert the game disc.

## Language Settings

When playing import game discs, the game may default to English as the console's language setting may not match the disc's region. To override the language, create a `disc-launcher.cnf` file in the same location as `disc-launcher.elf` with the following content:
```ini
# Language setting for disc launcher
# Valid values: 0=Japanese, 1=English, 2=French, 3=Spanish
# 4=German, 5=Italian, 6=Dutch, 7=Portuguese
# Default value if not set or invalid will be console's default
language = 1
```

## Notes

The Disc Launcher checks the region of both the PlayStation game disc and the console. If a mismatch is found (e.g., a PAL game in an NTSC console), then PS1VModeNeg is launched to adjust the video mode of the PlayStation driver. If no mismatch is found, the game is launched normally. PS1VModeNeg can be downloaded [here](https://github.com/ps2homebrew/PS1VModeNeg). `PS1VModeNeg.elf` should be placed in the same location as `disc-launcher.elf` (either on Memory Card (PS2), USB, or HDD). If `PS1VModeNeg.elf` is not found, then all PlayStation games will be launched normally.

## Credits

Based on [Retro GEM Disc Launcher](https://github.com/CosmicScale/Retro-GEM-PS2-Disc-Launcher) by [CosmicScale](https://github.com/CosmicScale).  
Updated by [Exalandru](https://github.com/corin-alex).