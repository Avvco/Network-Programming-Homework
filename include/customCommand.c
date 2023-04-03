typedef struct{
  char *name;
  void* (*preCommand)();
  void* (*processCommand)(void *commandRequirement);
} CustomCommand;

#include "./customCommand/who.c"

CustomCommand customCommand[] = {
  {"who", whoPreCommand, whoProcessCommand}
};