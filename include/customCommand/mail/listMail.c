typedef struct{
  char *receiverId;
} ListMail;

// mailto <user_name> message
void* listMailPreCommand(char *command, Session *session) {
  ListMail *listMail = malloc(sizeof(ListMail));
  listMail -> receiverId = session -> loggedInUserId;
  return (void *)listMail;
}

void* listMailProcessCommand(void *commandRequirement) { 
  ListMail *current = (ListMail *)commandRequirement;
  PGresult *res = getMailByUserId(current -> receiverId);
  if(res == NULL) {
    strcat(previousCommandOutput, "Empty!");
    return NULL;
  }
  int rows = PQntuples(res);
  char *returnStr = malloc(102400 * sizeof(char));
  memset(returnStr, 0, sizeof(returnStr));
  snprintf(returnStr, 1024000, "%-8s%-25s%-20s%-60s\n", "<ID>", "<DATE>", "<SENDER>", "<MESSAGE>");
  for(int i = 0 ; i < rows ; i++) {
    char *currentStr = malloc(10240 * sizeof(char));
    char *mailId = PQgetvalue(res, i, 0);
    char *senderName = getNameByUserId(PQgetvalue(res, i, 1));
    char *content = PQgetvalue(res, i, 3);
    char *date = PQgetvalue(res, i, 4);
    date[19] = '\0';
    snprintf(currentStr, 10240, "%-8s%-25s%-20s%-60s\n", mailId, date, senderName, content);
    strcat(returnStr, currentStr);
    free(currentStr);
  }
  strcpy(previousCommandOutput, returnStr);
}