#!/bin/bash

rm -f ./obj/out.out
gcc ./src/server.c -o ./obj/out.out
./obj/out.out
rm -f ./obj/out.out