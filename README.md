# Network-Programming-Homework
Spring 2022 Network-Programming, NDHU

## Prerequisite
You need to have:
- [Docker Compose](https://docs.docker.com/compose/) installed
- [inotify-tools](https://github.com/inotify-tools/inotify-tools) installed
- [tmux](https://github.com/tmux/tmux) installed
## How to run
```bash
sudo bash run.sh
```
## Services
### Server
- URL <http://127.0.0.1:64550>
### Adminer
- URL <http://127.0.0.1:64549>
- System: `PostgreSQL`
- Server: `postgres`
- Username: `postgres`
- Password: `password`
- Database: leave it empty
### RedisInsight
- URL <http://127.0.0.1:64548>
- Procedule
    1. Choose I already have a database
    2. Connect to a redis Database
    3. Fill the following content
- Connection properties
    - Host: `redis`
    - Port: `6379`
    - Name: `localhost`
    - Username: `default`
    - Password: `password`
    - No TLS
