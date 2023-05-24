typedef struct{
  char *name;
  void* (*preCommand)();
  void* (*processCommand)(void *commandRequirement);
} CustomCommand;

#include "./customCommand/who.c"
#include "./customCommand/name.c"
#include "./customCommand/yell.c"
#include "./customCommand/tell.c"
#include "./customCommand/mailto.c"
#include "./customCommand/listMail.c"
#include "./customCommand/delMail.c"

CustomCommand customCommand[] = {
  {"who", whoPreCommand, whoProcessCommand},
  {"name", namePreCommand, nameProcessCommand},
  {"yell", yellPreCommand, yellProcessCommand},
  {"tell", tellPreCommand, tellProcessCommand},
  {"mailto", mailToPreCommand, mailToProcessCommand},
  {"listMail", listMailPreCommand, listMailProcessCommand},
  {"delMail", delMailPreCommand, delMailProcessCommand}
};