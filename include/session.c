typedef struct{
  int id; // socket id
  int assignedId; // assigned id
  char *ip;
  int port;
  char *name;

  int isNextPassword; // 1 if next input is password
  char *providedUsername;
  char *providedPassword;
  int loginMode; // 0: default, 1: username, 2: password, 3: login success
  
  int pipeCounter;
  char *savedCommandOutput;
  char *previousCommandOutput;
} Session;

Session *session;