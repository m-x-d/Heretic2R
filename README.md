# Heretic2R

Heretic2R is Heretic II (1998, Raven Software) reverse-engineered source port.

## Features

* Widescreen support (with automatic HUD scaling).
* Rendering framerate decoupled from network packets sending rate (with theoretical maximum of 1000 FPS).
* OGG music playback.
* Most of special effects are updated at rendering framerate (instead of 20 FPS).
* Improved map loading times. 
* Lots of cosmetic improvements (so the game plays as you remember it, not as it actually played). 
* Many bugfixes.

## Installation

Heretic2R requires Heretic II game data in order to run. You can either:  
– Overwrite Heretic II binaries with the Heretic2R ones.  
**or**  
– Copy "**Heretic II\base**" folder (excluding all .dll files) to Heretic2R folder.

To enable OGG music playback, rip Heretic II CD tracks as **track02.ogg - track14.ogg** and place them in "**base\music**" folder.

## Technical notes

* H2R savegames/configs/screenshots/logs are stored in "**%USERPROFILE%\Saved Games\Heretic2R**".
* Singleplayer works, and is relatively well tested. The game is completable without major issues.
* Coop/multiplayer works, but is practically untested.
* By default, H2R uses network protocol 55 (H2 protocol 51 is also supported, but not recommended). Can be changed using "protocol" cvar.
* H2R savegames are **NOT** compatible with H2 savegames.
* H2R is **NOT** compatible with H2 renderers (because of API changes).
* H2R is **NOT** compatible with H2 sound backends (because of API changes).
* H2R is **NOT** compatible with H2 gamex86/Player/Client Effects libraries (because of API changes).
* Gamepad support is currently not implemented.
* HiDPI support is currently not implemented.
* Framerates above 60 FPS are not tested.
* Screen resolutions above FullHD are not tested.

## Planned features

* OpenGL 3 renderer.
* OpenAL sound backend.
* Gamepad support.

## Used libraries

* [glad](https://glad.dav1d.de)
* [libsmacker](https://github.com/JonnyH/libsmacker)
* [SDL3](https://www.libsdl.org)
* [stb](https://github.com/nothings/stb) (specifically, stb_image_write and stb_vorbis)
 
 ## SAST Tools

[PVS-Studio](https://pvs-studio.com/pvs-studio/?utm_source=website&utm_medium=github&utm_campaign=open_source) - static analyzer for C, C++, C#, and Java code.
