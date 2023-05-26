typedef struct{
  char *senderId;
  char *groupName;
  char *errorMessage;
} LeaveGroup;

void* leaveGroupPreCommand(char *command, Session *session) {
  LeaveGroup *leaveGroup = malloc(sizeof(LeaveGroup));
  leaveGroup -> senderId = session -> loggedInUserId;
  if(parseFrom1dTo2d(command)[1] == NULL) {
    leaveGroup -> errorMessage = "leaveGroup: not enough argument.";
    return (void *)leaveGroup;
  }
  leaveGroup -> groupName = strchr(command, ' ') + 1; // seprate the group name

  // remove the last space, unknow why it is there
  if(strcmp(&leaveGroup -> groupName[strlen(leaveGroup -> groupName) - 1], " ") == 0) leaveGroup -> groupName[strlen(leaveGroup -> groupName) - 1] = '\0';

  leaveGroup -> errorMessage = NULL;
  return (void *)leaveGroup;
}

void* leaveGroupProcessCommand(void *commandRequirement) {
  LeaveGroup *current = (LeaveGroup *)commandRequirement;
  if(current -> errorMessage != NULL) {
    strcpy(previousCommandOutput, current -> errorMessage);
    return NULL;
  }

  int result = leaveGroup(current -> groupName, current -> senderId);
  if(result == 1) {
    strcpy(previousCommandOutput, "leaveGroup: you are leaved the group.");
  }else if(result == 0) {
    strcpy(previousCommandOutput, "leaveGroup: you are not in the group.");
  }else {
    strcpy(previousCommandOutput, "leaveGroup: error");
  }
}