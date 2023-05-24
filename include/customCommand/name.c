typedef struct{
  int currentId;
  char **newName;
  char *userId;
} Name;

void* namePreCommand(char *command, Session *session) {
  Name *name = malloc(sizeof(Name));
  name -> currentId = session -> assignedId;
  name -> newName = parseFrom1dTo2d(command);
  name -> userId = session -> loggedInUserId;
  return (void *)name;
}

void* nameProcessCommand(void *commandRequirement) { 
  Name *current = (Name *)commandRequirement;
  char *returnStr = malloc(10240 * sizeof(char));
  if(current -> newName[1] == NULL) {
    strcpy(returnStr, "name: not enough argument.");
  }else if(strlen(current -> newName[1]) > 16) {
    strcpy(returnStr, "name is too long.");
  }else if(strcmp(current -> newName[1], " ") == 0) {
    strcpy(returnStr, "name cannot be empty.");
  }else if(current -> newName[2] != NULL) {
    strcpy(returnStr, "name: too many argument.");
  }else {
    // need to copy the string because otherwise the name will not be correct after existsByUsername, reason unknown.
    char *newName = malloc(8192 * sizeof(char));
    strcpy(newName, current -> newName[1]);
    if(existsByUsername(newName) == 0) {
      setNameById(current -> userId, newName);
      strcpy(returnStr, "name change accept.");
    }else {
      strcpy(returnStr, "name already in used, choose another name.");
    }
  }
  strcat(previousCommandOutput, returnStr);
}