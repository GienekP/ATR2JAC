# ATR2JAC
Convert ATARI ATR-files to CAR J(atari)Cart format

ATR Formats:
- 128 sector
- 256 sector
- SD
- DD
- CUSTOM ; max SD Size: 474368bytes (3706 sectors); max DD Size: 942336bytes (3681 sectors)

use:

atr2jac file.atr file.car [-c] [-d] [-128|-256|-512]

CAR  format "75" is resized to Atarimax 1MB(new) for emulators

-128 binary format for 128kB flash

-256 binary format for 256kB flash

-512 binary format for 512kB flash

-c remap JSR & JMP with JDSKINT / DSKINT


Examples:

a) conver ATR file to CAR (code 75) - Atarimax 1MB(new)

atr2jac game.atr game.car


b) convert ATR to binary for real FLASH 512kB based on Atarimax 1MB(new) standard

atr2jac game.atr game.bin -512


c) convert ATR to CAR and change all jumps to DSKINT ROM function

atr2jac game.atr game.car -c


e) convert ATR to binary for FLASH 256kB

atr2jac game.atr game.bin -c -256

