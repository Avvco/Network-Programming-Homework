#define PREVIOUS_COMMAND_OUTPUT_SIZE 32768
#define die(e) do { fprintf(stderr, "%s\n", e); exit(EXIT_FAILURE); } while (0);

char existBin[100][100];
int existBinCount = 0;
char previousCommandOutput[PREVIOUS_COMMAND_OUTPUT_SIZE];

char *prefix = "% ";
char *welcome = "Welcome to the shell!\nType 'quit' or 'exit' to exit the shell.\n";
char *prompt = "Welcome, type \"0\" to login, \"1\" to register, \"quit\" or \"exit\" to leave.\n";

char quit[] = "quit";
char _exitTerm[] = "exit";

#include "../include/runCommand.c"
#include "../include/customCommand.c"

void initShell() {
  //printf("Welcome to the shell!\n");
  //printf("Type 'quit' or 'exit' to exit the shell.\n");

  // scan the bin directory for executables, use it if exist, or else use the system executables.
  DIR *d;
  struct dirent *dir;
  d = opendir("./bin");
  if (d) {
    while ((dir = readdir(d)) != NULL) {
      if(dir->d_type == DT_REG) {
        strcpy(existBin[existBinCount], dir->d_name);
        existBinCount++;
        // printf("%s\n", dir->d_name);
      }
    }
    closedir(d);
  }
  memset(previousCommandOutput, 0, PREVIOUS_COMMAND_OUTPUT_SIZE);

  // mark this line if you want to use the system executables.
  setenv("PATH", "/bin:./bin", 1);
}


/**
 * @brief process the command, and return the execution status 
 * 
 * @param command command to be executed
 * @param session session of the client
 * @param envp env of the process
 * @return status of the command, -1 means exit, 0 means normal, 1 means error 
 */
int process(char *command, Session *session, char **envp) {
  char _setenv[] = "setenv";
  char printenv[] = "printenv";
  char *currentFirstCommand = malloc(128 * sizeof(char));

  char **splitedCommand = malloc(MAX_COMMANDS_SIZE * sizeof(char *));

  int isInternalCommand = 0;
  int piped = 0;

  command = removeLeading(command);
  if(strcmp(command, "\n") == 0 || strcmp(command, "\r\n") == 0 || strcmp(command, "\r") == 0) return 1;

  int splitedCommandCount = command_parse(command, splitedCommand);
  int currentCommandIndex = 0;
  for (int i = 0; i < splitedCommandCount; i++) {
    trim_command(splitedCommand[i]); 
  }

  if(strcmp(splitedCommand[0], quit) == 0 || strcmp(splitedCommand[0], _exitTerm) == 0) return -1;

  /**
   * Because the setenv in child process will not affect the parent process, 
   * so setenv can't be implemented by directly calling run_bash.
   * 
   * And printenv can't be implemented by directly calling either,  because we override the PATH,
   * causing printenv in system executables not found.
   * 
   * printenv and setenv only work when it is at the beginning of the command, but support pipe.
   * 
   * Save the output of printenv to previousCommandOutput, so that it can be used in the next command.
   */
  if(strcmp(splitedCommand[0], printenv) == 0) {
    if(splitedCommandCount < 2 || strcmp(splitedCommand[1], "|") == 0) {
      currentCommandIndex += 2;
      memset(previousCommandOutput, 0, PREVIOUS_COMMAND_OUTPUT_SIZE);
      for (char **env = envp; *env != 0; env++) {
        char *thisEnv = *env;
        strcat(previousCommandOutput, thisEnv);
        strcat(previousCommandOutput, "\n");
      }
    }
    else if(splitedCommandCount > 2 && strcmp(splitedCommand[2], "|") != 0) {
      printf("printenv: too many arguments\n");
      memset(previousCommandOutput, 0, PREVIOUS_COMMAND_OUTPUT_SIZE);
      return 0;
    }
    else {
      currentCommandIndex += 3;
      memset(previousCommandOutput, 0, PREVIOUS_COMMAND_OUTPUT_SIZE);
      strcat(previousCommandOutput, getenv(splitedCommand[1]));
    }
    if(strcmp(&previousCommandOutput[strlen(previousCommandOutput) - 1], "\n") == 0) previousCommandOutput[strlen(previousCommandOutput) - 1] = '\0';
  }else if(strcmp(splitedCommand[0], _setenv) == 0) {
    if(splitedCommandCount < 3) printf("setenv: not enough arguments\n");
    else if(splitedCommandCount > 3) printf("setenv: too many arguments\n");
    else setenv(splitedCommand[1], splitedCommand[2], 1);
    currentCommandIndex += 3;
  }
  for(int i = currentCommandIndex ; i < splitedCommandCount ; i++) {
    // check if it is an internal command
    for(int j = 0; j < existBinCount; j++) {
      if(strcmp(splitedCommand[i], existBin[j]) == 0) { isInternalCommand = 1; break; }
      else isInternalCommand = 0;
    }

    /**
     * Build the command
     * 
     * Pipe all previous command output to the current command,
     * it doesn't take effect if there is no previous command output or the current command is not accept input.
     */
    char *currentCommand = malloc(MAX_COMMANDS_SIZE * sizeof(char));

    /**
     * Pipe counter:
     * 0: no pipe
     * 1: current command is piped
     * >1: command output is queued to be piped
     */
    if(session -> pipeCounter <= 0) {
      session -> pipeCounter = 0;
    }else if(session -> pipeCounter > 1) {
      session -> pipeCounter--;
    }else if(session -> pipeCounter == 1) {
      session -> pipeCounter--;
      memset(previousCommandOutput, 0, PREVIOUS_COMMAND_OUTPUT_SIZE);
      strcat(previousCommandOutput, session -> savedCommandOutput);
      memset(session -> savedCommandOutput, 0, PREVIOUS_COMMAND_OUTPUT_SIZE); 
    }

    /**
     * Add "./bin/" in front of the command if it is an internal command and 
     * consider all input till "|" or the end of the line is the argument of the command
     */
    if(isInternalCommand) strcat(currentCommand, "./bin/");
    int counter = 0;
    while(i < splitedCommandCount && strstr(splitedCommand[i], "|") == NULL) {
      if(counter == 0) strcpy(currentFirstCommand, splitedCommand[i]);
      strcat(currentCommand, splitedCommand[i]);
      strcat(currentCommand, " ");
      i++;
      counter++;
    }

    // printf("currentCommand: %s\n", currentCommand);

    /**
     * Check if the command is a custom command then build the requirement for the command and run.
     */
    int isCustomCommand = 0;
    for(int j = 0 ; j < sizeof(customCommand) / sizeof(customCommand[0]) ; j++) {
      if(strcmp(customCommand[j].name, currentFirstCommand) == 0) {
        isCustomCommand = 1;
        void *commandRequirement = customCommand[j].preCommand(currentCommand, session);
        customCommand[j].processCommand(commandRequirement);
        free(commandRequirement);
        break;
      }
    }

    if(!isCustomCommand) {
      run_command(currentCommand);
    }
    
    // remove the last line break
    if(strcmp(&previousCommandOutput[strlen(previousCommandOutput) - 1], "\n") == 0) previousCommandOutput[strlen(previousCommandOutput) - 1] = '\0';


    /**
     * Check if the command contains pipe
     * If present, save the output of the command to session -> savedCommandOutput.
     */
    if(splitedCommand[i] != NULL) {
      char *pipCount = strstr(splitedCommand[i], "|");
      if(pipCount != NULL) {
        if(strlen(pipCount) > 1) {
          session -> pipeCounter = atoi(&pipCount[1]);
        }
        else session -> pipeCounter = 1;
        memset(session -> savedCommandOutput, 0, PREVIOUS_COMMAND_OUTPUT_SIZE);
        strcat(session -> savedCommandOutput, previousCommandOutput);
        piped = 1;
      }
    }
  }

  /**
   * If command not found, length of previousCommandOutput will be 0.
   * If the command is piped, not print to the console
   * If the next command is the piped command, override the previousCommandOutput with session -> savedCommandOutput
   * Return the result of command to client only if it is not piped or piped but the current command is not piped
   * 
   * Pipcounter:
   * 0: no pipe
   * 1: next command is piped
   * >1: ignore
   */
  // printf("previousCommandOutput: %d\n", strlen(previousCommandOutput));
  if(strlen(previousCommandOutput) > 0) {
    if(session -> pipeCounter == 0 || (session -> pipeCounter != 0 && !piped)) {
      strcat(session -> previousCommandOutput, previousCommandOutput);
      memset(previousCommandOutput, 0, PREVIOUS_COMMAND_OUTPUT_SIZE);
    }else {
      memset(previousCommandOutput, 0, PREVIOUS_COMMAND_OUTPUT_SIZE);
      return 1;
    }
  }else {
    // command not found will not consume session -> pipeCounter
    strcat(session -> previousCommandOutput, currentFirstCommand);
    strcat(session -> previousCommandOutput, ": command not found");
    if(session -> pipeCounter != 0) session -> pipeCounter++;
  }
  return 0;
}