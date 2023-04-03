typedef struct{
  int id; // socket id
  int assignedId; // assigned id
  char *ip;
  int port;
  char *name;
  
  int pipeCounter;
  char *savedCommandOutput;
  char *previousCommandOutput;
  int mode;
} Session;

Session *session;