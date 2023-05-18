#include <hiredis/hiredis.h>
#include <libpq-fe.h>

const char *REDIS_IP = "redis"; // dns resolved by docker network
const int REDIS_PORT = 6379;
const char *REDIS_PASSWORD = "password";

const char *POSTGRE_IP = "postgre"; // dns resolved by docker network
const int POSTGRE_PORT = 5432;
const char *POSTGRE_USERNAME = "postgres";
const char *POSTGRE_PASSWORD = "password";

/**
 * @brief execute a redis command and return the output, not use redis as database anymore
 * 
 * @param command the redis command to be executed
 * @param output the output of the command
 * @return 1 if success with value, 0 if success but without value, -1 if fail
 * @deprecated
 */
int doOnRedis(char *command, char **output) {
  *output = malloc(256 * sizeof(char));
  redisContext *conn = redisConnect(REDIS_IP, REDIS_PORT);
  if (conn == NULL || conn->err) { 
    if (conn) {
      strcpy(*output, "Error: ");
      strcat(*output, conn->errstr);
      strcat(*output, "\n");
    }else {
      strcpy(*output, "Can't allocate redis context\n");
    }
    return -1;
  }
  redisReply *reply = redisCommand(conn, "AUTH %s", REDIS_PASSWORD);
  freeReplyObject(reply);
  reply = redisCommand(conn, command);

  // if not found, return will be null
  if(reply->str == NULL) {
    strcpy(*output, "null");
    return 0;
  }else {
    strcpy(*output, reply->str);
    redisFree(conn); 
    return 1;
  }
}


int doOnPostgre() {

}