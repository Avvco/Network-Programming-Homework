typedef struct{
  char *senderId;
  char *groupName;
  char *errorMessage;
} DelGroup;

void* delGroupPreCommand(char *command, Session *session) {
  DelGroup *delGroup = malloc(sizeof(DelGroup));
  delGroup -> senderId = session -> loggedInUserId;
  if(parseFrom1dTo2d(command)[1] == NULL) {
    delGroup -> errorMessage = "delGroup: not enough argument.";
    return (void *)delGroup;
  }
  delGroup -> groupName = strchr(command, ' ') + 1; // seprate the group name

  // remove the last space, unknow why it is there
  if(strcmp(&delGroup -> groupName[strlen(delGroup -> groupName) - 1], " ") == 0) delGroup -> groupName[strlen(delGroup -> groupName) - 1] = '\0';

  delGroup -> errorMessage = NULL;
  return (void *)delGroup;
}

void* delGroupProcessCommand(void *commandRequirement) {
  DelGroup *current = (DelGroup *)commandRequirement;
  if(current -> errorMessage != NULL) {
    strcpy(previousCommandOutput, current -> errorMessage);
    return NULL;
  }

  int result = delGroup(current -> groupName, current -> senderId);
  if(result == 1) {
    strcpy(previousCommandOutput, "delGroup: group deleted.");
  }else if(result == 0) {
    strcpy(previousCommandOutput, "delGroup: you are not owner.");
  }else {
    strcpy(previousCommandOutput, "delGroup: error");
  }
}