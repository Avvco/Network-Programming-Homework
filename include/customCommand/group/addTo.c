typedef struct{
  char *senderId;
  char *groupName;
  char *toAdd;
  char *errorMessage;
} AddTo;

void* addToPreCommand(char *command, Session *session) {
  AddTo *addTo = malloc(sizeof(AddTo));
  addTo -> senderId = session -> loggedInUserId;
  if(parseFrom1dTo2d(command)[1] == NULL || parseFrom1dTo2d(command)[2] == NULL) {
    addTo -> errorMessage = "addTo: not enough argument.\n";
    return (void *)addTo;
  }
  addTo -> groupName = malloc(1024 * sizeof(char));
  strcpy(addTo -> groupName, parseFrom1dTo2d(command)[1]);
  addTo -> toAdd = malloc(2048 * sizeof(char *));
  addTo -> toAdd = strchr(strchr(command, ' ') + 1, ' ') + 1;

  addTo -> errorMessage = NULL;
  return (void *)addTo;
}

void* addToProcessCommand(void *commandRequirement) {
  AddTo *current = (AddTo *)commandRequirement;
  if(current -> errorMessage != NULL) {
    strcpy(previousCommandOutput, current -> errorMessage);
    return NULL;
  }
  fprintf(stderr, "addTo: groupName: %d\n", existGroupByName(current -> groupName));
  if(existGroupByName(current -> groupName) != 1) {
    strcpy(previousCommandOutput, "addTo: group not exist.\n");
    return NULL;
  } else if(isGroupOwner(current -> groupName, current -> senderId) != 1) {
    strcpy(previousCommandOutput, "addTo: you are not the owner of the group.\n");
    return NULL;
  }
  int index = 0;
  char *returnStr = malloc(10240 * sizeof(char));
  memset(returnStr, 0, sizeof(returnStr));

  while(parseFrom1dTo2d(current -> toAdd)[index] != NULL) {
    char *currentStr = malloc(1024 * sizeof(char));
    memset(currentStr, 0, sizeof(currentStr));
    // fprintf(stderr, "toAdd[%d]: %s\n", index, parseFrom1dTo2d(current -> toAdd)[index]);
    char *userId = getIdByUsername(parseFrom1dTo2d(current -> toAdd)[index]);
    if(userId == NULL) {
      snprintf(currentStr, 1024, "addTo: user '%s' not exist.\n", parseFrom1dTo2d(current -> toAdd)[index]);
    }else {
      int result = addUserToGroup(current -> groupName, userId);
      if(result == 1) {
        snprintf(currentStr, 1024, "addTo: user '%s' already in group.\n", parseFrom1dTo2d(current -> toAdd)[index]);
      }else if(result == 0) {
        snprintf(currentStr, 1024, "addTo: user '%s' added.\n", parseFrom1dTo2d(current -> toAdd)[index]);
      }else {
        strcpy(currentStr, "addTo: error.\n");
      }
    }
    index++;
    strcat(returnStr, currentStr);
  }
  strcpy(previousCommandOutput, returnStr);
}