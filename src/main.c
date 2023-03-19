#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dirent.h> 
#include <ctype.h>

#define PREVIOUS_COMMAND_OUTPUT_SIZE 32768
#define die(e) do { fprintf(stderr, "%s\n", e); exit(EXIT_FAILURE); } while (0);

char existBin[100][100];
int existBinCount = 0;
char previousCommandOutput[PREVIOUS_COMMAND_OUTPUT_SIZE];

// use for number pipe
char savedCommandOutput[PREVIOUS_COMMAND_OUTPUT_SIZE];

#include "../include/commandParse.c"
#include "../include/runCommand.c"

void init() {
  printf("Welcome to the shell!\n");
  printf("Type 'quit' or 'exit' to exit the shell.\n");

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

int main(int argc, char **argv, char **envp) {
  char command[MAX_COMMANDS_SIZE];
  char quit[] = "quit";
  char _exit[] = "exit";
  char _setenv[] = "setenv";
  char printenv[] = "printenv";

  int pipeCounter = 0;

  init();
  while (1) {
    char **splitedCommand = malloc(MAX_COMMANDS_SIZE * sizeof(char *));
    int isInternalCommand = 0;
    int piped = 0;
    printf("%s", "% ");
    
    if(fgets(command, MAX_COMMANDS_SIZE, stdin) == NULL) {
      printf("\n");
      return 0;
    }
    if(strcmp(command, "\n") == 0) continue;
    int splitedCommandCount = command_parse(command, splitedCommand);
    int currentCommandIndex = 0;
    for (int i = 0; i < splitedCommandCount; i++) {
      trim_command(splitedCommand[i]);
    }

    if(strcmp(splitedCommand[0], quit) == 0 || strcmp(splitedCommand[0], _exit) == 0) return 0;

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
        continue;
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
      if(pipeCounter <= 0) {
        pipeCounter = 0;
      }else if(pipeCounter >= 1) {
        pipeCounter--;
      }

      /**
       * Add "./bin/" in front of the command if it is an internal command and 
       * consider all input till "|" or the end of the line is the argument of the command
       */
      if(isInternalCommand) strcat(currentCommand, "./bin/");
      while(i < splitedCommandCount && strstr(splitedCommand[i], "|") == NULL) {
        strcat(currentCommand, splitedCommand[i]);
        strcat(currentCommand, " ");
        i++;
      }

      // printf("currentCommand: %s\n", currentCommand);
      run_command(currentCommand);


      /**
       * Check if the command contains pipe
       * If present, save the output of the command to savedCommandOutput.
       */
      if(splitedCommand[i] != NULL) {
        char *pipCount = strstr(splitedCommand[i], "|");
        if(pipCount != NULL) {
          if(strlen(pipCount) > 1) {
            pipeCounter = atoi(&pipCount[1]);
          }
          else pipeCounter = 1;
          memset(savedCommandOutput, 0, PREVIOUS_COMMAND_OUTPUT_SIZE);
          strcat(savedCommandOutput, previousCommandOutput);
          piped = 1;
        }
      }
    }

    /**
     * If command not found, length of previousCommandOutput will be 0.
     * If the command is piped, not print to the console
     * If the next command is the piped command, override the previousCommandOutput with savedCommandOutput
     * 
     * Pipcounter:
     * 0: no pipe
     * 1: next command is piped
     * >1: ignore
     */
    // printf("previousCommandOutput: %d\n", strlen(previousCommandOutput));
    if(strlen(previousCommandOutput) > 0) {
      if(pipeCounter == 0) printf("%s\n", previousCommandOutput);
      if(pipeCounter != 0 && !piped) printf("%s\n", previousCommandOutput);
      if(pipeCounter == 1) {
        memset(previousCommandOutput, 0, PREVIOUS_COMMAND_OUTPUT_SIZE);
        strcat(previousCommandOutput, savedCommandOutput);
        memset(savedCommandOutput, 0, PREVIOUS_COMMAND_OUTPUT_SIZE);
      }else {
        memset(previousCommandOutput, 0, PREVIOUS_COMMAND_OUTPUT_SIZE);
      }
    }else {
      // command not found will not consume pipeCounter
      if(pipeCounter != 0) pipeCounter++;
    }
  }
}