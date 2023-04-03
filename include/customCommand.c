typedef struct{
  char *name;
  void* (*preCommand)();
  void* (*processCommand)(void *commandRequirement);
} CustomCommand;

#include "./customCommand/who.c"
#include "./customCommand/name.c"

CustomCommand customCommand[] = {
  {"who", whoPreCommand, whoProcessCommand},
  {"name", namePreCommand, nameProcessCommand}
};