#define MAX_COMMANDS_SIZE 5000
#include <ctype.h>

/**
 * @brief trim the command by remove space and line breaks
 * 
 * @param str command to trim
 */
void trim_command(char *str) {
  // remove line breaks with both crlf and lf
  strtok(str, "\r\n");
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

/**
 * @brief remove leading spaces
 * 
 * @param str string to remove leading spaces
 * @return the string without leading spaces
 */
char *removeLeading(char *str) {
  int idx = 0, j, k = 0;
  char *str1 = malloc(MAX_COMMANDS_SIZE * sizeof(char));
  // Iterate String until last
  // leading space character
  while(str[idx] == ' ' || str[idx] == '\t' || str[idx] == '\n') {
    idx++;
  }

  // Run a for loop from index until the original
  // string ends and copy the content of str to str1
  for(j = idx; str[j] != '\0'; j++) {
    str1[k] = str[j];
    k++;
  }

  // Insert a string terminating character
  // at the end of new string
  
  // prevent start and only with \t
  if(k == 0) {
    str1[1] = '\0';
    str1[0] = '\n';
    return str1;
  }
  str1[k] = '\0';
  return str1;
  // Print the string with no whitespaces
  // printf("%s", str1);
}

char **parseFrom1dTo2d(const char *command) {
  char cpcommand[MAX_COMMANDS_SIZE];

  char **_command = malloc(MAX_COMMANDS_SIZE * sizeof(char *));

  strcpy(cpcommand, command);
  command_parse(cpcommand, _command);
  return _command;
}