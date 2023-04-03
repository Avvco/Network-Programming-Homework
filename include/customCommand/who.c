typedef struct{
  int currentId;
} Who;

void* whoPreCommand(char *command, Session *session) {
  Who *who = malloc(sizeof(Who));
  who -> currentId = session -> assignedId;
  return (void *)who;
}

void* whoProcessCommand(void *commandRequirement) {
  Who *current = (Who *)commandRequirement;
  char *prefixStr = malloc(256 * sizeof(char));
  sprintf(prefixStr, "%-5s%-20s%-20s%-5s\n", "<ID>", "<NAME>", "<IP:PORT>", "<INDECATOR>");
  char *returnStr = malloc(10240 * sizeof(char));
  strcpy(returnStr, prefixStr);
  for(int i = 0 ; i < MAX_CLIENTS ; i++) {
    for(int j = 0 ; j < MAX_CLIENTS ; j++) {
      if(session[j].assignedId == i) {
        char *currentStr = malloc(256 * sizeof(char));
        char *ipAndPort = malloc(32 * sizeof(char));
        strcpy(ipAndPort, session[j].ip);
        strcat(ipAndPort, ":");
        char *port = malloc(16 * sizeof(char));
        sprintf(port, "%d", session[j].port);
        strcat(ipAndPort, port);
        sprintf(currentStr, "%-5d%-20s%-20s%-5s\n", i, session[j].name, ipAndPort, (current -> currentId == i) ? "<- (ME)" : "");
        strcat(returnStr, currentStr);
        free(currentStr);
        free(ipAndPort);
        free(port);
        break;
      }
    }
  }
  strcat(previousCommandOutput, returnStr);
}