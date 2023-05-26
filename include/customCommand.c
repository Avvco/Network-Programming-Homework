typedef struct{
  char *name;
  void* (*preCommand)();
  void* (*processCommand)(void *commandRequirement);
} CustomCommand;

#include "./customCommand/who.c"
#include "./customCommand/name.c"
#include "./customCommand/yell.c"
#include "./customCommand/tell.c"
#include "./customCommand/mail/mailto.c"
#include "./customCommand/mail/listMail.c"
#include "./customCommand/mail/delMail.c"
#include "./customCommand/group/createGroup.c"
#include "./customCommand/group/addTo.c"
#include "./customCommand/group/delGroup.c"
#include "./customCommand/group/leaveGroup.c"
#include "./customCommand/group/remove.c"
#include "./customCommand/group/listGroup.c"
#include "./customCommand/group/gyell.c"


CustomCommand customCommand[] = {
  {"who", whoPreCommand, whoProcessCommand},
  {"name", namePreCommand, nameProcessCommand},
  {"yell", yellPreCommand, yellProcessCommand},
  {"tell", tellPreCommand, tellProcessCommand},
  {"mailto", mailToPreCommand, mailToProcessCommand},
  {"listMail", listMailPreCommand, listMailProcessCommand},
  {"delMail", delMailPreCommand, delMailProcessCommand},
  {"createGroup", createGroupPreCommand, createGroupProcessCommand},
  {"addTo", addToPreCommand, addToProcessCommand},
  {"delGroup", delGroupPreCommand, delGroupProcessCommand},
  {"leaveGroup", leaveGroupPreCommand, leaveGroupProcessCommand},
  {"remove", removeFromGroupPreCommand, removeFromGroupProcessCommand},
  {"listGroup", listGroupPreCommand, listGroupProcessCommand},
  {"gyell", gyellPreCommand, gyellProcessCommand},
};