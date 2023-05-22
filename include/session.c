typedef struct{
  int id; // socket id
  int assignedId; // assigned id
  char *ip;
  int port;
  char *name;
  char *loggedInUserId;

  char *providedUsername;
  char *providedPassword;
  int mode; // -2: re entry, -1: default, 0: login, 1: register, 2: logged in
  int inputMode; // -1: no input, 0: input is username, 1: input is password
  
  int pipeCounter;
  char *savedCommandOutput;
  char *previousCommandOutput;
} Session;

Session *session;