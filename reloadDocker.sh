#!/bin/bash

docker stop -t 0 $(docker ps -a -q)
rm -f *.s
docker system prune -f
docker-compose up -d