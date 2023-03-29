typedef struct{
  int id;
  int pipeCounter;
  char *savedCommandOutput;
  char *previousCommandOutput;
  int mode;
} Session;
