# Jam entry

# Building

## Using docker

```
docker build . -t n64brew2025
docker run -v ./:/n64brew2025 -it --rm n64brew2025 make
```

## Not using docker

Install the preview branch of libdragon [https://github.com/DragonMinded/libdragon/wiki/Installing-libdragon](https://github.com/DragonMinded/libdragon/wiki/Installing-libdragon)

Install tiny3d [https://github.com/HailToDodongo/tiny3d](https://github.com/HailToDodongo/tiny3d)

Download blender 4.5.8 somewhere on your system [https://download.blender.org/release/Blender4.5/](https://download.blender.org/release/Blender4.5/)
Then set the environment variable BLENDER_4 to be where the blender executable is located

run

`make`

# Making changes to the game

The build system exports files and levels from blend files. 

## wav/mp3 files

You can place a .txt file with the same name as an mp3 or wav in the same directory and put flags for audioconv64 that apply to that sound. Options are 

```
WAV/MP3 options:
   --wav-mono                   Force mono output
   --wav-resample <N>           Resample to a different sample rate
   --wav-compress <0|1|3>       Enable compression: 0=none, 1=vadpcm (default), 3=opus
   --wav-loop <true|false>      Activate playback loop by default
   --wav-loop-offset <N>        Set looping offset (in samples; default: 0)
   --wav-seek <SEC|FILE>        Enable seeking support:
                                - if SEC is a float, add a seekpoint every SEC seconds
                                - if FILE, read a list of seekpoints (one per line):
                                  * integer sample offsets, or
                                  * timestamps in [hh:]mm:ss[.mmm] format
```