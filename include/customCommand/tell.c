typedef struct{
  int currentId;
  char *toTell;
  int destinationId;
} Tell;

void* tellPreCommand(char *command, Session *session) {
  Tell *tell = malloc(sizeof(Tell));
  tell -> currentId = session -> assignedId;
  tell -> toTell = strchr(command, ' ') + 1;
  return (void *)tell;
}

void* tellProcessCommand(void *commandRequirement) {
  Tell *current = (Tell *)commandRequirement;
  char *returnStr = malloc(10240 * sizeof(char));
  if(current -> toTell[0] == '\0') {
    strcpy(returnStr, "yell: not enough argument.\n");
  }else {
    int destinationId = atoi((parseFrom1dTo2d(current -> toTell)[0]));
    int destinationExist = 0;
    for(int i = 0 ; i < MAX_CLIENTS ; i++) {
      if(session[i].assignedId == destinationId) {
        destinationExist = 1;
        break;
      }
    }
    if(destinationExist == 1) {
      // cut destinationId out
      char *_toTell = strchr(current -> toTell, ' ') + 1;
      fprintf(stderr, "toTell: %s\n", _toTell);
      if(_toTell[0] == '\0') {
        strcpy(returnStr, "tell: not enough argument.\n");
      }else {
        snprintf(returnStr, 10240, "tell accept to %d: %s", destinationId,_toTell);
        char *__toTell = malloc(10240 * sizeof(char));
        snprintf(__toTell, 10240, "<user(%d) told you>: %s\n%s", current -> currentId, _toTell, prefix);
        for(int i = 0 ; i < MAX_CLIENTS ; i++) {
          if(session[i].assignedId == destinationId) {
            send(session[i].id, __toTell, strlen(__toTell), 0);
          }
        }
      }
    }else {
      strcpy(returnStr, "tell: invalid destination id.\n");
    }
  }
  strcat(previousCommandOutput, returnStr);
}