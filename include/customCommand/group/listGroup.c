typedef struct{
  char *senderId;
} ListGroup;


void* listGroupPreCommand(char *command, Session *session) {
  ListGroup *listGroup = malloc(sizeof(ListGroup));
  listGroup -> senderId = malloc(1024 * sizeof(char));
  strcpy(listGroup -> senderId, session -> loggedInUserId);
  return (void *)listGroup;
}

void* listGroupProcessCommand(void *commandRequirement) { 
  ListGroup *current = (ListGroup *)commandRequirement;
  PGresult *res = listGroupByUserId(current -> senderId);
  if(res == NULL) {
    strcat(previousCommandOutput, "Empty!");
    return NULL;
  }
  int rows = PQntuples(res);
  char *returnStr = malloc(102400 * sizeof(char));
  memset(returnStr, 0, sizeof(returnStr));
  snprintf(returnStr, 1024000, "%-10s%-15s\n", "<OWNER>", "<GROUP>");
  for(int i = 0 ; i < rows ; i++) {
    char *currentStr = malloc(10240 * sizeof(char));
    char *groupName = PQgetvalue(res, i, 1);
    char *owner = getNameByUserId(PQgetvalue(res, 0, 2));
    snprintf(currentStr, 10240, "%-10s%-15s\n", owner, groupName);
    strcat(returnStr, currentStr);
    free(currentStr);
  }
  strcpy(previousCommandOutput, returnStr);
}