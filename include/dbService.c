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
  // fprintf(stderr, "Command %s\n", command);
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
  char *createUserTable = "CREATE TABLE IF NOT EXISTS network.user (                    \
                            user_id uuid DEFAULT uuid_generate_v4(),                    \
                            name VARCHAR(64) NOT NULL,                                  \
                            password VARCHAR(64) NOT NULL,                              \
                            PRIMARY KEY(user_id)                                        \
                          );";
  char *createMailTable = "CREATE TABLE IF NOT EXISTS network.mail (                    \
                            mail_id SERIAL,                                             \
                            sender_id uuid NOT NULL,                                    \
                            receiver_id uuid NOT NULL,                                  \
                            content VARCHAR(10240) NOT NULL,                            \
                            data_time timestamp NOT NULL DEFAULT NOW(),                 \
                            PRIMARY KEY(mail_id),                                       \
                            FOREIGN KEY(sender_id) REFERENCES network.user(user_id),    \
                            FOREIGN KEY(receiver_id) REFERENCES network.user(user_id)   \
                          );";
  char *createGroupTable = "CREATE TABLE IF NOT EXISTS network.group (                  \
                              group_id uuid DEFAULT uuid_generate_v4(),                 \
                              name VARCHAR(64) NOT NULL,                                \
                              owner_id uuid NOT NULL,                                   \
                              member_id uuid[],                                         \
                              PRIMARY KEY(group_id),                                    \
                              FOREIGN KEY(owner_id) REFERENCES network.user(user_id)    \
                            );";
  if( doOnPostgre(createExtension)  == -1 || 
      doOnPostgre(createSchema)     == -1 || 
      doOnPostgre(createUserTable)  == -1 ||
      doOnPostgre(createMailTable)  == -1 ||
      doOnPostgre(createGroupTable) == -1
    ) {
    return -1;
  }

  fprintf(stderr, "Finish postgres initialization\n");
  return 0;
}

/**
 * @brief print the query result in table format
 * 
 * @param output the postgres query result
 */
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



/**
 * @brief login to the system, fill the session logged in user id if success
 * 
 * @param session 
 * @return 1 if success, 0 if password or username incorrect, -1 if execution fail
 */
int login(Session *session) {
  PGresult *output = NULL;
  // fprintf(stderr, "%s\n", session -> providedUsername);
  // fprintf(stderr, "%s\n", session -> providedPassword);
  
  char *command = malloc(10240 * sizeof(char));
  
  snprintf(command, 10240, "SELECT * FROM network.user WHERE name = '%s' AND password = '%s';", session -> providedUsername, session -> providedPassword);
  // fprintf(stderr, "%s\n", command);
  if(doOnPostgreWithOutput(command, &output) == -1) {
    return -1;
  }
  // printTable(output);
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

/**
 * @brief find user with the name in database
 * 
 * @param userName 
 * @return int 0 if not found, 1 if found, -1 if fail
 */
int existsByUsername(char *userName) {
  PGresult *output;
  char *command = malloc(10240 * sizeof(char));
  snprintf(command, 10240, "SELECT * FROM network.user WHERE name = '%s';", userName);
  if(doOnPostgreWithOutput(command, &output) == -1) {
    return -1;
  }
  if(PQntuples(output) == 0) {
    return 0;
  }
  //printTable(output);
  PQclear(output);
  return 1;
}

/**
 * @brief Set the Name of user with id
 * 
 * @param userId 
 * @param newName 
 * @return int 0 if success, -1 if fail
 */
int setNameById(char *userId, char *newName) {
  PGresult *output;
  // fprintf(stderr, "Set name of user %s to %s\n", userId, newName);
  char *command = malloc(10240 * sizeof(char));
  snprintf(command, 10240, "UPDATE network.user SET name = '%s' WHERE user_id = '%s';", newName, userId);
  if(doOnPostgreWithOutput(command, &output) == -1) {
    return -1;
  }
  PQclear(output);
  return 0;
}

/**
 * @brief Add new mail into database
 * 
 * @param senderId 
 * @param receiverId 
 * @param content 
 * @return 0 if success, -1 if fail
 */
int insertMail(char *senderId, char *receiverId, char *content) {
  PGresult *output;
  char *command = malloc(10240 * sizeof(char));
  snprintf(command, 10240, "INSERT INTO network.mail (sender_id, receiver_id, content) VALUES ('%s', '%s', '%s');", senderId, receiverId, content);
  if(doOnPostgreWithOutput(command, &output) == -1) {
    return -1;
  }
  PQclear(output);
  return 0;
}

/**
 * @brief Get the Mail By User Id 
 * 
 * @param userId 
 * @return the mail of user, NULL if empty
 */
PGresult *getMailByUserId(char *userId) {
  PGresult *output;
  char *command = malloc(10240 * sizeof(char));
  snprintf(command, 10240, "SELECT * FROM network.mail WHERE receiver_id = '%s';", userId);
  if(doOnPostgreWithOutput(command, &output) == -1) {
    return NULL;
  }
  if(PQntuples(output) == 0) {
    return NULL;
  }
  return output;
}

/**
 * @brief Create new group
 * 
 * @param newName 
 * @param ownerId 
 * @return 1 if already exist, 0 if success, -1 if fail 
 */
int newGroup(char *newName, char *ownerId) {
  if(existGroupByName(newName) == 1) {
    return 1;
  }
  PGresult *output;
  char *command = malloc(10240 * sizeof(char));
  snprintf(command, 10240, "INSERT INTO network.group (name, owner_id, member_id) VALUES ('%s', '%s', '{%s}');", newName, ownerId, ownerId);
  if(doOnPostgreWithOutput(command, &output) == -1) {
    return -1;
  }
  PQclear(output);
  return 0;
}

/**
 * @brief Add user to group
 * 
 * @param groupName 
 * @param userId 
 * @return 1 if already exist, 0 if not, -1 if fail 
 */
int addUserToGroup(char *groupName, char *userId) {
  if(existGroupByName(groupName) == 0) {
    return -1;
  }
  if(existUserInGroup(groupName, userId) == 1) {
    return 1;
  }
  PGresult *output;
  char *command = malloc(10240 * sizeof(char));
  snprintf(command, 10240, "UPDATE network.group SET member_id = array_append(member_id, '%s') WHERE name = '%s';", userId, groupName);
  if(doOnPostgreWithOutput(command, &output) == -1) {
    return -1;
  }
  PQclear(output);
  return 0;
}

/**
 * @brief Remove user from group
 * 
 * @param groupName 
 * @param userId 
 * @return 1 if already exist, 0 if success, -1 if fail 
 */
int removeUserFromGroup(char *groupName, char *userId) {
  if(existGroupByName(groupName) == 0) {
    return -1;
  }
  PGresult *output;
  char *command = malloc(10240 * sizeof(char));
  snprintf(command, 10240, "UPDATE network.group SET member_id = array_remove(member_id, '%s') WHERE name = '%s';", userId, groupName);
  if(doOnPostgreWithOutput(command, &output) == -1) {
    return -1;
  }
  PQclear(output);
  return 0;
}

/**
 * @brief Check if user is in group
 * 
 * @param groupName 
 * @param userId 
 * @return 1 if already in group, 0 if not, -1 if fail
 */
int existUserInGroup(char *groupName, char *userId) {
  PGresult *output;
  char *command = malloc(10240 * sizeof(char));
  snprintf(command, 10240, "SELECT * FROM network.group WHERE name = '%s' AND member_id @> '{%s}';", groupName, userId);
  if(doOnPostgreWithOutput(command, &output) == -1) {
    return -1;
  }
  if(PQntuples(output) == 0) {
    return 0;
  }
  PQclear(output);
  return 1;
}


/**
 * @brief Get the Group By Group name
 * 
 * @param groupName 
 * @return 0 if not found, 1 if found, -1 if fail
 */
int existGroupByName(char *groupName) {
  PGresult *output;
  char *command = malloc(10240 * sizeof(char));
  snprintf(command, 10240, "SELECT * FROM network.group WHERE name = '%s';", groupName);
  if(doOnPostgreWithOutput(command, &output) == -1) {
    return -1;
  }
  if(PQntuples(output) == 0) {
    return 0;
  }
  PQclear(output);
  return 1;
}

/**
 * @brief Get the Group By User Id 
 * 
 * @param userId 
 * @return 1 if is owner, 0 if not, -1 if fail
 */
int isGroupOwner(char *groupName, char *userId) {
  PGresult *output;
  char *command = malloc(10240 * sizeof(char));
  snprintf(command, 10240, "SELECT * FROM network.group WHERE name = '%s' AND owner_id = '%s';", groupName, userId);
  if(doOnPostgreWithOutput(command, &output) == -1) {
    return -1;
  }
  if(PQntuples(output) == 0) {
    return 0;
  }
  PQclear(output);
  return 1;
}

/**
 * @brief Delete group
 * 
 * @param groupName 
 * @param userId 
 * @return 1 if success, 0 if not owner, -1 if fail
 */
int delGroup(char *groupName, char *userId) {
  if(isGroupOwner(groupName, userId) != 1) {
    return 0;
  }
  PGresult *output;
  char *command = malloc(10240 * sizeof(char));
  snprintf(command, 10240, "DELETE FROM network.group WHERE name = '%s';", groupName);
  if(doOnPostgreWithOutput(command, &output) == -1) {
    return -1;
  }
  PQclear(output);
  return 1;
}

/**
 * @brief Leave group
 * 
 * @param groupName 
 * @param userId 
 * @return 1 if success, 0 if not in group, -1 if fail 
 */
int leaveGroup(char *groupName, char *userId) {
  if(existUserInGroup(groupName, userId) != 1) {
    return 0;
  }
  if(isGroupOwner(groupName, userId) == 1) {
    return delGroup(groupName, userId);
  }
  PGresult *output;
  char *command = malloc(10240 * sizeof(char));
  snprintf(command, 10240, "UPDATE network.group SET member_id = array_remove(member_id, '%s') WHERE name = '%s';", userId, groupName);
  if(doOnPostgreWithOutput(command, &output) == -1) {
    return -1;
  }
  PQclear(output);
  return 1;
}


/**
 * @brief Remove user from group by owner
 * 
 * @param groupName 
 * @param userId 
 * @param ownerId 
 * @return 1 if success, 0 if not in group, -1 if fail, -2 if not owner
 */
int removeUserFromGroupByOwner(char *groupName, char *userId, char *ownerId) {
  if(isGroupOwner(groupName, ownerId) != 1) {
    return -2;
  }
  return leaveGroup(groupName, userId);
}

/**
 * @brief Get the Group By User Id
 * 
 * @param userId 
 * @return the group if success, NULL if empty
 */
PGresult *listGroupByUserId(char *userId) {
  PGresult *output;
  char *command = malloc(10240 * sizeof(char));
  snprintf(command, 10240, "SELECT * FROM network.group WHERE member_id @> '{%s}';", userId);
  if(doOnPostgreWithOutput(command, &output) == -1) {
    return NULL;
  }
  if(PQntuples(output) == 0) {
    return NULL;
  }
  // printTable(output);
  return output;
}

/**
 * @brief Get the Group By Group name
 * 
 * @param groupName 
 * @return the group if success, NULL if empty
 */
PGresult *listGroupByGroupName(char *groupName) {
  PGresult *output;
  char *command = malloc(10240 * sizeof(char));
  snprintf(command, 10240, "SELECT * FROM network.group WHERE name = '%s';", groupName);
  if(doOnPostgreWithOutput(command, &output) == -1) {
    return NULL;
  }
  if(PQntuples(output) == 0) {
    return NULL;
  }
  // printTable(output);
  return output;
}

/**
 * @brief Get all member in group if user in the group
 * @param groupName
 * @param finder the user that exist in group
 * @param member if success, NULL if empty
 * @return 1 if found, 0 if not found, -1 if fail
 */
int listMemberByGroupName(char *groupName, char *finder, char **member) {
  PGresult *output;
  char *command = malloc(10240 * sizeof(char));
  snprintf(command, 10240, "SELECT member_id FROM network.group WHERE name = '%s';", groupName);
  if(doOnPostgreWithOutput(command, &output) == -1) {
    return -1;
  }
  if(PQntuples(output) == 0 || strstr(PQgetvalue(output, 0, 0), finder) == NULL) {
    return 0;
  }
  int num = PQntuples(output);
  // fprintf(stderr, "%d\n", PQntuples(output));
  strcpy(*member, PQgetvalue(output, 0, 0));
  PQclear(output);
  return num;
}

/**
 * @brief Get the Name By User Id 
 * 
 * @param userId 
 * @return the name of user, NULL if fail
 */
char *getNameByUserId(char *userId) {
  PGresult *output;
  char *command = malloc(10240 * sizeof(char));
  snprintf(command, 10240, "SELECT name FROM network.user WHERE user_id = '%s';", userId);
  if(doOnPostgreWithOutput(command, &output) == -1) {
    return NULL;
  }
  if(PQntuples(output) == 0) {
    return NULL;
  }
  char *name = malloc(2048 * sizeof(char));
  strcpy(name, PQgetvalue(output, 0, 0));
  PQclear(output);
  return name;
}

/**
 * @brief Get the Id By Username object
 * 
 * @param username 
 * @return id of user, NULL if fail
 */
char *getIdByUsername(char *username) {
  PGresult *output;
  char *command = malloc(10240 * sizeof(char));
  snprintf(command, 10240, "SELECT user_id FROM network.user WHERE name = '%s';", username);
  if(doOnPostgreWithOutput(command, &output) == -1) {
    return NULL;
  }
  if(PQntuples(output) == 0) {
    return NULL;
  }
  char *id = malloc(2048 * sizeof(char));
  strcpy(id, PQgetvalue(output, 0, 0));
  PQclear(output);
  return id;
}

/**
 * @brief Get the Mail with userName and Id
 * 
 * @return the id of mail, NULL if not found or fail  
 */
char *getMailWithUsernameAndId(char *userId, char *mailId) {
  PGresult *output;
  char *command = malloc(10240 * sizeof(char));
  snprintf(command, 10240, "SELECT * FROM network.mail WHERE receiver_id = '%s' AND mail_id = '%s';", userId, mailId);
  if(doOnPostgreWithOutput(command, &output) == -1) {
    return NULL;
  }
  char *id = malloc(2048 * sizeof(char));
  if(PQntuples(output) == 0) {
    return NULL;
  }else {
    strcpy(id, PQgetvalue(output, 0, 0));
  }
  return id;
}

/**
 * @brief Delete the Mail By Id
 * 
 * @param mailId 
 * @return 0 if success, -1 if fail
 */
int deleteMailWithId(char *mailId) {
  PGresult *output;
  char *command = malloc(10240 * sizeof(char));
  snprintf(command, 10240, "DELETE FROM network.mail WHERE mail_id = '%s';", mailId);
  if(doOnPostgreWithOutput(command, &output) == -1) {
    return -1;
  }
  PQclear(output);
  return 0;
}