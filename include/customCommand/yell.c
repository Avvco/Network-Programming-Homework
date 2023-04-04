typedef struct{
  int currentId;
  char *toYell;
} Yell;

void* yellPreCommand(char *command, Session *session) {
  Yell *yell = malloc(sizeof(Yell));
  yell -> currentId = session -> assignedId;
  yell -> toYell = strchr(command, ' ') + 1;
  return (void *)yell;
}

void* yellProcessCommand(void *commandRequirement) {
  Yell *current = (Yell *)commandRequirement;
  char *returnStr = malloc(10240 * sizeof(char));
  //fprintf(stderr, "command: %s  \n", current -> toYell);
  if(current -> toYell[0] == '\0') {
    strcpy(returnStr, "yell: not enough argument.\n");
  }else {
    strcpy(returnStr, "yell accept: ");
    strcat(returnStr, current -> toYell);
    char *_toYell = malloc(10240 * sizeof(char));
    snprintf(_toYell, 10240, "<user(%d) yelled>: %s\n%s", current -> currentId, current -> toYell, prefix);
    for(int i = 0 ; i < MAX_CLIENTS ; i++) {
      if(session[i].assignedId != current -> currentId) {
        if(session[i].id != -1) {
          send(session[i].id, _toYell, strlen(_toYell), 0);
        }
      }
    }
  }
  
 
  strcat(previousCommandOutput, returnStr);
}