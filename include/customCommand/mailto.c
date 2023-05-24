typedef struct{
  char *senderId;
  char *receiverName;
  char *content;
  char *errorMessage;
} MailTo;

// mailto <user_name> message
void* mailToPreCommand(char *command, Session *session, char **envp) {
  MailTo *mailto = malloc(sizeof(MailTo));
  mailto -> senderId = session -> loggedInUserId;
  if(parseFrom1dTo2d(command)[1] == NULL || parseFrom1dTo2d(command)[2] == NULL) {
    mailto -> errorMessage = "mailto: not enough argument.\n";
    return (void *)mailto;
  }
  // if direct point to the content, then the content will be lost, reason unknown.
  mailto -> receiverName = malloc(128 * sizeof(char));
  strcpy(mailto -> receiverName, parseFrom1dTo2d(command)[1]); // seprate the username

  if(strcmp(parseFrom1dTo2d(command)[2], "<") == 0) { // for case that the content is redirect from command, eg: mailto <user_name> < ls
    char *content = malloc(1024 * sizeof(char));
    // seprate the content to the third space
    
    if(strchr(strchr(strchr(command, ' ') + 1, ' ') + 1, ' ') + 1) {
      content = strchr(strchr(strchr(command, ' ') + 1, ' ') + 1, ' ') + 1;
    }else {
      mailto -> errorMessage = "mailto: not enough argument.\n";
      return (void *)mailto;
    }
    fprintf(stderr, "content: %s\n", content);

    // generate new session for the command
    Session newSession;
    newSession.id = -1; 
    newSession.assignedId = -1;
    newSession.pipeCounter = 0;
    newSession.savedCommandOutput = malloc(PREVIOUS_COMMAND_OUTPUT_SIZE * sizeof(char));
    newSession.previousCommandOutput = malloc(PREVIOUS_COMMAND_OUTPUT_SIZE * sizeof(char));
    process(content, &newSession, envp);
    mailto -> content = newSession.previousCommandOutput;
  }else {
    // seprate the content by cut the first space and the second space
    mailto -> content = strchr(strchr(command, ' ') + 1, ' ') + 1;
  }
  
  fprintf(stderr, "senderId: %s\n", mailto -> senderId);
  fprintf(stderr, "receiverName: %s\n", mailto -> receiverName);
  fprintf(stderr, "content: %s\n", mailto -> content);
  mailto -> errorMessage = NULL;
  return (void *)mailto;
}

void* mailToProcessCommand(void *commandRequirement) { 
  MailTo *current = (MailTo *)commandRequirement;
  if(current -> errorMessage != NULL) {
    strcat(previousCommandOutput, current -> errorMessage);
    return NULL;
  }
  char *receverId = getIdByUsername(current -> receiverName);

  if(receverId == NULL) {
    strcat(previousCommandOutput, "receiver not found.");
    return NULL;
  }

  // insert the mail to the database
  insertMail(current -> senderId, receverId, current -> content);

  char *returnStr = malloc(10240 * sizeof(char));

  // snprintf(returnStr, 10240, "senderId: %s\nreceiverName: %s\ncontent: %s\n", current -> senderId, current -> receiverName, current -> content);
  snprintf(returnStr, 10240, "Send accept!");

  strcat(previousCommandOutput, returnStr);
}