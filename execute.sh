#!/bin/bash

if gcc ./src/server.c -o /tmp/out.out -lhiredis -I/usr/include/postgresql -lpq ; then
  /wait && /tmp/out.out
fi
