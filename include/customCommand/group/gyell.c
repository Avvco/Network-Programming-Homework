typedef struct{
  char *senderId;
  int senderSocketId;
  char *groupName;
  char *toYell;
  char *errorMessage;
} GYell;

void* gyellPreCommand(char *command, Session *session) {
  GYell *gyell = malloc(sizeof(GYell));
  gyell -> senderId = session -> loggedInUserId;
  gyell -> toYell = strchr(command, ' ') + 1;
  gyell -> senderSocketId = session -> id;
  if(parseFrom1dTo2d(command)[1] == NULL || parseFrom1dTo2d(command)[2] == NULL) {
    gyell -> errorMessage = "gyell: not enough argument.\n";
    return (void *)gyell;
  }
  gyell -> groupName = malloc(1024 * sizeof(char));
  strcpy(gyell -> groupName, parseFrom1dTo2d(command)[1]); // seprate the group name

  gyell -> toYell = strchr(strchr(command, ' ') + 1, ' ') + 1; // seprate the content by cut the first space and the second space
  gyell -> errorMessage = NULL;
  return (void *)gyell;
}

void* gyellProcessCommand(void *commandRequirement) {
  GYell *current = (GYell *)commandRequirement;
  if(current -> errorMessage != NULL) {
    strcat(previousCommandOutput, current -> errorMessage);
    return NULL;
  }
  char *member = malloc(10240 * sizeof(char));
  int memberCount = listMemberByGroupName(current -> groupName, current -> senderId, &member); // {member1, member2, member3}
  if(memberCount == 0) {
    strcat(previousCommandOutput, "gyell: group not found.\n");
    return NULL;
  }
  // remove first and last char of member
  member[strlen(member) - 1] = '\0';
  member++;
  
  int index = 0;
  char *delim = ",";
  while(parseFrom1dTo2dWithDelim(member, delim)[index]) {
    char *currentId = malloc(256 * sizeof(char));
    strcpy(currentId, parseFrom1dTo2dWithDelim(member, delim)[index]);
    fprintf(stderr, "memberList: %s\n", currentId);
    for(int i = 0 ; i < MAX_CLIENTS ; i++) {
      if(session[i].id != -1 && 
         strcmp(session[i].loggedInUserId, currentId) == 0 &&
         strcmp(session[i].loggedInUserId, current -> senderId) != 0
        ) {
        char *_toYell = malloc(10240 * sizeof(char));
        // <group_name:user_name>: content
        snprintf(_toYell, 10240, "<%s:%s>: %s\n(%s)%s", current -> groupName, getNameByUserId(current -> senderId), current -> toYell, getNameByUserId(session[i].loggedInUserId), prefix);
        send(session[i].id, _toYell, strlen(_toYell), 0);
      }
    }
    index++;
  }
  
  char *returnStr = malloc(10240 * sizeof(char));
  snprintf(returnStr, 10240, "gyell accept to %s: %s\n", current -> groupName, current -> toYell);
  strcpy(previousCommandOutput, returnStr);
}