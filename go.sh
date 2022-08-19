#!/bin/sh

gcc -Wall -o SectorMap SectorMap.c
./SectorMap

./mads starter128j.asm -o:starter128j.bin
./mads starter256j.asm -o:starter256j.bin

xxd -cols 16 -i starter128j.bin > starter128j.h
xxd -cols 16 -i starter256j.bin > starter256j.h

gcc -Wall -o atr2jac atr2jac.c
./atr2jac
