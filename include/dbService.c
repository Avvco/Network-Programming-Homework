#include <hiredis/hiredis.h>
#include <libpq-fe.h>

const char *REDIS_IP = "redis"; // dns resolved by docker network
const int REDIS_PORT = 6379;
const char *REDIS_PASSWORD = "password";

const char *POSTGRES_IP = "postgres"; // dns resolved by docker network
const int POSTGRES_PORT = 5432;
const char *POSTGRES_USERNAME = "postgres";
const char *POSTGRES_PASSWORD = "password";

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


/**
 * @brief execute a postgre command and return the output
 * 
 * @param command 
 * @return 0 if success, -1 if fail 
 */
int doOnPostgreWithOutput(char *command, PGresult **output) {  
  char *conninfo = malloc(256 * sizeof(char));
  sprintf(conninfo, "host=%s port=%d user=%s password=%s dbname=%s", POSTGRES_IP, POSTGRES_PORT, POSTGRES_USERNAME, POSTGRES_PASSWORD, POSTGRES_USERNAME);
  PGconn *conn = PQconnectdb(conninfo);
  fprintf(stderr, "Command %s\n", command);
  if (PQstatus(conn) == CONNECTION_BAD) {        
    fprintf(stderr, "Can't connect to Postgres: %s\n", PQerrorMessage(conn));
    PQfinish(conn);
    exit(-1);
  }
  *output = PQexec(conn, command);
  if (PQresultStatus(*output) != PGRES_COMMAND_OK && PQresultStatus(*output) != PGRES_TUPLES_OK) { 
    fprintf(stderr, "Failed to execute command: %s with reason %s\n", command, PQresultErrorMessage(*output));
    PQfinish(conn);
    return -1;
  }

  PQfinish(conn);
  // fprintf(stderr, "Close connection to database\n");
  return 0;
}

int doOnPostgre(char *command) {
  PGresult *output;
  return doOnPostgreWithOutput(command, &output);
}

/**
 * @brief initialize postgre database
 * 
 * @return int 0 if success, -1 if fail
 */
int initPostgre() {
  char *createExtension = "CREATE EXTENSION IF NOT EXISTS \"uuid-ossp\";";
  char *createSchema = "CREATE SCHEMA IF NOT EXISTS network;";
  char *createUserTable = "CREATE TABLE IF NOT EXISTS network.user (                  \
                            user_id uuid DEFAULT uuid_generate_v4(),                  \
                            name VARCHAR(64) NOT NULL,                                \
                            password VARCHAR(64) NOT NULL,                            \
                            PRIMARY KEY(user_id)                                      \
                          );";
  char *createMailTable = "CREATE TABLE IF NOT EXISTS network.mail (                  \
                            mail_id uuid DEFAULT uuid_generate_v4 (),                 \
                            sender_id uuid NOT NULL,                                  \
                            receiver_id uuid NOT NULL,                                \
                            subject VARCHAR(64) NOT NULL,                             \
                            content VARCHAR(1024) NOT NULL,                           \
                            PRIMARY KEY(mail_id),                                     \
                            FOREIGN KEY(sender_id) REFERENCES network.user(user_id),  \
                            FOREIGN KEY(receiver_id) REFERENCES network.user(user_id) \
                          );";
  if( doOnPostgre(createExtension)  == -1 || 
      doOnPostgre(createSchema)     == -1 || 
      doOnPostgre(createUserTable)  == -1 ||
      doOnPostgre(createMailTable)  == -1
    ) {
    return -1;
  }

  fprintf(stderr, "Finish postgres initialization\n");
  return 0;
}



/**
 * @brief login to the system, fill the session logged in user id if success
 * 
 * @param session 
 * @return 1 if success, 0 if password or username incorrect, -1 if execution fail
 */
int login(Session *session) {
  PGresult *output = NULL;
  fprintf(stderr, "%s\n", session -> providedUsername);
  fprintf(stderr, "%s\n", session -> providedPassword);
  
  char *command = malloc(10240 * sizeof(char));
  
  snprintf(command, 10240, "SELECT * FROM network.user WHERE name = '%s' AND password = '%s';", session -> providedUsername, session -> providedPassword);
  fprintf(stderr, "%s\n", command);
  if(doOnPostgreWithOutput(command, &output) == -1) {
    return -1;
  }
  printTable(output);
  if(PQntuples(output) == 0) {
    return 0;
  }
  session -> loggedInUserId = malloc(2048 * sizeof(char));
  strcpy(session -> loggedInUserId, PQgetvalue(output, 0, 0));
  strcpy(session -> name, PQgetvalue(output, 0, 1));
  PQclear(output);
  fprintf(stderr, "Logged in user id: %s\n", session -> loggedInUserId);
  return 1;
}

/**
 * @brief 
 * 
 * @param userName 
 * @param password 
 * @return int 0 if success, -1 if fail 
 */
int registerUser(char *userName, char *password) {
  PGresult *output;
  char *command = malloc(10240 * sizeof(char));
  snprintf(command, 10240, "INSERT INTO network.user (name, password) VALUES ('%s', '%s');", userName, password);
  if(doOnPostgreWithOutput(command, &output) == -1) {
    return -1;
  }
  PQclear(output);
  return 0;
}

// function to print postgres table
void printTable(PGresult *output) {
  fprintf(stderr, "Table with row: %d, column: %d\n", PQntuples(output), PQnfields(output));
  // print column name
  for (int i = 0; i < PQnfields(output); i++) {
    fprintf(stderr, "%s ", PQfname(output, i));
  }
  fprintf(stderr, "\n");
  // print column values
  for (int i = 0; i < PQntuples(output); i++) {
    for (int j = 0; j < PQnfields(output); j++) {
      fprintf(stderr, "%s ", PQgetvalue(output, i, j));
    }
    fprintf(stderr, "\n");
  }
}