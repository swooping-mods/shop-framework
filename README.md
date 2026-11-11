# Shop Framework

Separate native Modded Shop with real registered item purchases.

## Install

Install [Moon Sweepers Mod Loader](https://github.com/swooping-mods/moon-sweepers-mod-loader) first.

Download [shop-framework-0.2.1.msmod](content/shop-framework-0.2.1.msmod), import it with **Install mod package**, then select the mod and its required frameworks.

## Controls

- Ctrl+Shift+F2 opens the separate Modded Shop using the normal native shop UI.
- Vanilla shops retain vanilla rows; registered mod items have real price, ownership and Buy actions.
- Framework owns the shortcut once; Modded Shop entry is optional. Gameplay checks deferred.

## Compatibility

Version 0.2.1. Targets the pinned Steam Windows x64 IL2CPP build of Moon Sweepers (Steam app 5335510). On Linux, the game runs through Proton. Experimental: back up saves before using save-affecting mods.

## Build

Run `python3 build.py --check` to verify source hashes, then `python3 build.py` to write a build under `.local/build/`. Requires installed Clang, lld-link and llvm-dlltool.

## Credits

See [CREDITS.txt](CREDITS.txt) and [third-party notices](vendored/THIRD_PARTY_NOTICES.md).
