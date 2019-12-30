#!/bin/bash
flex cminus.l
bison -d cminus.y
gcc -c *.c -g
gcc -o main *.o -ly -lfl -g

