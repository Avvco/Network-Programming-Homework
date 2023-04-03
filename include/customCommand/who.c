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
  sprintf(prefixStr, "%-8s%-20s%-20s%-20s\n", "<ID>", "<NAME>", "<IP:PORT>", "<INDECATOR>");
  char *returnStr = malloc(10240 * sizeof(char));
  strcpy(returnStr, prefixStr);
  for(int i = 0 ; i < MAX_CLIENTS ; i++) {
    for(int j = 0 ; j < MAX_CLIENTS ; j++) {
      if(session[j].assignedId == i) {
        char *currentStr = malloc(256 * sizeof(char));
        char *port = malloc(16 * sizeof(char));
        snprintf(port, 16,"%d", session[j].port);
        char *ipAndPort = malloc(32 * sizeof(char));
        snprintf(ipAndPort, 32, "%s:%s", session[j].ip, port);
        snprintf(currentStr, 256,"%-8d%-20s%-20s%-20s\n", i, session[j].name, ipAndPort, (current -> currentId == i) ? "<- (ME)" : "");
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