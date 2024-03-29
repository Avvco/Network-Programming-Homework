#!/bin/bash
chown -R 1001:1001 redis/
chown -R 1001 redisinsight/
./reloadDocker.sh
exec tmux new-session \; \
      send-keys './watch-change.sh' C-m \; \
      split-window -v -p 70\; \
      send-keys 'tail -f `docker inspect --format='{{.LogPath}}' server` | jq .log' C-m \; \
      select-pane -t 0 \; \