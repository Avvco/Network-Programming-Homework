#!/bin/bash

gcc ./src/server.c -o /tmp/out.out -lhiredis -I/usr/include/postgresql
/wait && /tmp/out.out