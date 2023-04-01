#!/bin/bash

docker stop -t 0 $(docker ps -a -q)
rm -f *.s
docker system prune -f
docker-compose up -d

exec tmux new-session \; \
      send-keys './watch-change.sh docker-compose restart server -t 0' C-m \; \
      split-window -v -p 70\; \
      send-keys 'tail -f `docker inspect --format='{{.LogPath}}' server` | jq .log' C-m \; \
      select-pane -t 0 \; \