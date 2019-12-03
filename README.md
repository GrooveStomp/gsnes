# GrooveStomp's NES Emulator
[![AGPLv3 license](https://img.shields.io/badge/License-AGPLv3-blue.svg)](https://raw.githubusercontent.com/GrooveStomp/gsnes/master/LICENSE) [![OLC-3 license](https://img.shields.io/badge/License-OLC&dash;3-blue.svg)](https://raw.githubusercontent.com/GrooveStomp/gsnes/master/LICENSE-OLC-3)

This NES emulator is based heavily off of the work done by [OneLoneCoder](https://github.com/OneLoneCoder/olcNES) ie., [javidx9](https://www.youtube.com/watch?v=F8kx56OZQhg).
I have written this emulator in C11 instead of the source C++17 used by javidx9.

## License
While this software is licensed under AGPLv3, this software _also_ retains the original license provided by OneLoneCoder.
Note that AGPLv3 is a much more _strict_ license than the OLC-3 license used by javidx9.

# Development
## Requirements
- make
- gcc
- sdl2
- doxygen (For documentation generation)

This is developed for Linux and no effort has been made to support it elsewhere.

## Building
There are four targets in the `Makefile`:
- `clean`
- `debug`
- `release`
- `docs`

The default target is `release`.
`release` builds `gstxt` at `release/gstxt`.
`debug` builds `gstxt` at `debug/gsxtxt`.
`docs` builds the documentation with Doxygen.

## Using
For now the emulator is not configurable outside of modifying source directly. The NES rom to load is hardcoded into main.c.

# Screenshots
![NES Test](/docs/screenshots/gsnes-2019-12-03.01.png?raw=true "NES Test")
![Donkey Kong](/docs/screenshots/gsnes-2019-12-03.06.png?raw=true "Donkey Kong")
![Ice Climber](/docs/screenshots/gsnes-2019-12-03.07.png?raw=true "Ice Climber")