docker stop -t 0 $(docker ps -a -q)
docker system prune -f
docker-compose up -d
./watch-change.sh docker-compose restart server -t 0