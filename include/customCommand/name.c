typedef struct{
  int currentId;
  char **newName;
} Name;

void* namePreCommand(char *command, Session *session) {
  Name *name = malloc(sizeof(Name));
  name -> currentId = session -> assignedId;
  name -> newName = parseFrom1dTo2d(command);
  return (void *)name;
}

void* nameProcessCommand(void *commandRequirement) { 
  Name *current = (Name *)commandRequirement;
  char *returnStr = malloc(10240 * sizeof(char));
  if(current -> newName[1] == NULL) {
    strcpy(returnStr, "name: not enough argument.\n");
  }else if(strlen(current -> newName[1]) > 16) {
    strcpy(returnStr, "name is too long.");
  }else if(strcmp(current -> newName[1], " ") == 0) {
    strcpy(returnStr, "name cannot be empty.");
  }else if(current -> newName[2] != NULL) {
    strcpy(returnStr, "name: too many argument.");
  }else {
    int nameIsTaken = 0;
    for(int i = 0 ; i < MAX_CLIENTS ; i++) {
      if(strcmp(session[i].name, current -> newName[1]) == 0) {
        strcpy(returnStr, "name is already taken.");
        nameIsTaken = 1;
        break;
      }
    }
    if(nameIsTaken == 0) {
      for(int i = 0 ; i < MAX_CLIENTS ; i++) {
        if(session[i].assignedId == current -> currentId) {
          memset(session[i].name, 0, 256);
          strcpy(session[i].name, current -> newName[1]);
          break;
        }
      }
      strcpy(returnStr, "name change accept.");
    }else {
      strcpy(returnStr, "name already in used, choose another name.");
    }
    
  }
  fprintf(stderr, "command: %s %s\n", current -> newName[0], current -> newName[1]);
  strcat(previousCommandOutput, returnStr);
}