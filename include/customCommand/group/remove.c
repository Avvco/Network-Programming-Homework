typedef struct{
  char *senderId;
  char *groupName;
  char *toAdd;
  char *errorMessage;
} RemoveFromGroup;

void* removeFromGroupPreCommand(char *command, Session *session) {
  RemoveFromGroup *removeFromGroup = malloc(sizeof(RemoveFromGroup));
  removeFromGroup -> senderId = session -> loggedInUserId;
  if(parseFrom1dTo2d(command)[1] == NULL || parseFrom1dTo2d(command)[2] == NULL) {
    removeFromGroup -> errorMessage = "removeFromGroup: not enough argument.\n";
    return (void *)removeFromGroup;
  }
  removeFromGroup -> groupName = malloc(1024 * sizeof(char));
  strcpy(removeFromGroup -> groupName, parseFrom1dTo2d(command)[1]);
  removeFromGroup -> toAdd = malloc(2048 * sizeof(char *));
  removeFromGroup -> toAdd = strchr(strchr(command, ' ') + 1, ' ') + 1;

  removeFromGroup -> errorMessage = NULL;
  return (void *)removeFromGroup;
}

void* removeFromGroupProcessCommand(void *commandRequirement) {
  RemoveFromGroup *current = (RemoveFromGroup *)commandRequirement;
  if(current -> errorMessage != NULL) {
    strcpy(previousCommandOutput, current -> errorMessage);
    return NULL;
  }
  fprintf(stderr, "removeFromGroup: groupName: %d\n", existGroupByName(current -> groupName));
  if(existGroupByName(current -> groupName) != 1) {
    strcpy(previousCommandOutput, "removeFromGroup: group not exist.\n");
    return NULL;
  } else if(isGroupOwner(current -> groupName, current -> senderId) != 1) {
    strcpy(previousCommandOutput, "removeFromGroup: you are not the owner of the group.\n");
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
    if(strcmp(userId, current -> senderId) == 0) {
      snprintf(returnStr, 1024, "removeFromGroup: you can't remove yourself from the group.\n");
    }else if(userId == NULL) {
      snprintf(currentStr, 1024, "removeFromGroup: user '%s' not exist.\n", parseFrom1dTo2d(current -> toAdd)[index]);
    }else {
      int result = removeUserFromGroupByOwner(current -> groupName, userId, current -> senderId);
      if(result == 0) {
        snprintf(currentStr, 1024, "removeFromGroup: user '%s' not in group.\n", parseFrom1dTo2d(current -> toAdd)[index]);
      }else if(result == 1) {
        snprintf(currentStr, 1024, "removeFromGroup: user '%s' removed.\n", parseFrom1dTo2d(current -> toAdd)[index]);
      }else if(result == -2) {
        strcpy(currentStr, "removeFromGroup: error.\n");
      }
    }
    index++;
    strcat(returnStr, currentStr);
  }
  strcpy(previousCommandOutput, returnStr);
}