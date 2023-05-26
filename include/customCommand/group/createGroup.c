typedef struct{
  char *senderId;
  char *groupName;
  char *errorMessage;
} CreateGroup;

void* createGroupPreCommand(char *command, Session *session) {
  CreateGroup *createGroup = malloc(sizeof(CreateGroup));
  createGroup -> senderId = session -> loggedInUserId;
  if(parseFrom1dTo2d(command)[1] == NULL) {
    createGroup -> errorMessage = "createGroup: not enough argument.";
    return (void *)createGroup;
  }
  createGroup -> groupName = strchr(command, ' ') + 1; // seprate the group name

  // remove the last space, unknow why it is there
  if(strcmp(&createGroup -> groupName[strlen(createGroup -> groupName) - 1], " ") == 0) createGroup -> groupName[strlen(createGroup -> groupName) - 1] = '\0';

  createGroup -> errorMessage = NULL;
  return (void *)createGroup;
}

void* createGroupProcessCommand(void *commandRequirement) {
  CreateGroup *current = (CreateGroup *)commandRequirement;
  if(current -> errorMessage != NULL) {
    strcpy(previousCommandOutput, current -> errorMessage);
    return NULL;
  }

  int result = newGroup(current -> groupName, current -> senderId);
  if(result == 1) {
    strcpy(previousCommandOutput, "createGroup: group name already exist.");
  }else if(result == 0) {
    strcpy(previousCommandOutput, "createGroup: group created.");
  }else {
    strcpy(previousCommandOutput, "createGroup: error");
  }
}