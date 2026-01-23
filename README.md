# Jam entry


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