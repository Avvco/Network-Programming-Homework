#define MAX_COMMANDS_SIZE 5000

/**
 * @brief trim the command by remove space and line breaks
 * 
 * @param str command to trim
 */
void trim_command(char *str) {
  // remove line breaks
  strtok(str, "\n");

  char *dst = str;
  for (; *str; ++str) {
    *dst++ = *str;
    if (isspace(*str)) {
      do ++str; 
      while (isspace(*str));
      --str;
    }
  }
  *dst = 0;
}

/**
 * @brief split the command with space
 * 
 * @param command command to split
 * @param args the array to store the splited command
 * @return the number of the split commands
 */
int command_parse(char *command, char **args) {
  int argc = 0;
  char *arg = malloc(MAX_COMMANDS_SIZE * sizeof(char));
  char *delim = " ";
  arg = strtok(command, delim);
  while (arg != NULL) {
    args[argc++] = arg;
    arg = strtok(NULL, delim);
  }
  args[argc] = NULL;
  return argc;
}