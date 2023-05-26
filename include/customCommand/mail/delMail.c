typedef struct{
  char *ownerId;
  char *mailId;
} DelMail;

// mailto <user_name> message
void* delMailPreCommand(char *command, Session *session) {
  DelMail *delMail = malloc(sizeof(DelMail));
  delMail -> ownerId = session -> loggedInUserId;
  if(parseFrom1dTo2d(command)[1]) {
    delMail -> mailId = parseFrom1dTo2d(command)[1];
  }else {
    delMail -> mailId = NULL;
    return NULL;
  }
  return (void *)delMail;
}

void* delMailProcessCommand(void *commandRequirement) { 
  DelMail *current = (DelMail *)commandRequirement;
  if(current -> mailId == NULL) {
    strcpy(previousCommandOutput, "delMail: not enough argument.\n");
    return NULL;
  }
  char *foundMail = getMailWithUsernameAndId(current -> ownerId, current -> mailId);
  if(foundMail) {
    deleteMailWithId(foundMail);
    strcpy(previousCommandOutput, "Delete accept");
    
  }else {
    strcpy(previousCommandOutput, "Mail not found\n");
  }
}