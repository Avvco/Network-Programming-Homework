#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dirent.h> 
#include <ctype.h>

#define die(e) do { fprintf(stderr, "stderr: %s\n", e); exit(EXIT_FAILURE); } while (0);
#define MAX_COMMANDS_SIZE 5000
#define PREVIOUS_COMMAND_OUTPUT_SIZE 32768
char existBin[100][100];
int existBinCount = 0;
char previousCommandOutput[PREVIOUS_COMMAND_OUTPUT_SIZE];

// use for number pipe
char savedCommandOutput[PREVIOUS_COMMAND_OUTPUT_SIZE];

int run_bash(const char *command) {
  int link[2];
  pid_t pid;
  char foo[4096 + 1];

  if (pipe(link) == -1)
    die("pipe");

   if ((pid = fork()) == -1)
    die("fork");

  if(pid == 0) {
    dup2 (link[1], STDOUT_FILENO);
    close(link[0]);
    close(link[1]);
    execl("/bin/bash", "bash", "-c", command, (char *)0);
    die("execl");
  } else {
    close(link[1]);
    int nbytes = 0;
    memset(previousCommandOutput, 0, PREVIOUS_COMMAND_OUTPUT_SIZE);
    while(0 != (nbytes = read(link[0], foo, sizeof(foo)))) {
      //printf("%.*s\n", nbytes, foo);
      strcat(previousCommandOutput, foo);
      memset(foo, 0, 4096);
    }
    if(strcmp(&previousCommandOutput[strlen(previousCommandOutput) - 1], "\n") == 0) previousCommandOutput[strlen(previousCommandOutput) - 1] = '\0';
    wait(NULL);
  }
}

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

int main(int argc, char **argv, char **envp) {
  char command[MAX_COMMANDS_SIZE];
  char quit[] = "quit";
  char _exit[] = "exit";
  char _setenv[] = "setenv";
  char printenv[] = "printenv";
  char command_not_found[] = "command not found";

  int pipeCount = 0;

  init();
  while (1) {
    char **splitedCommand = malloc(MAX_COMMANDS_SIZE * sizeof(char *));;
    int isInternalCommand = 0;
    printf("%s", "% ");
    if(fgets(command, MAX_COMMANDS_SIZE, stdin) == NULL) {
      return 0;
    }
    if(strcmp(command, "\n") == 0) continue;
    int splitedCommandCount = command_parse(command, splitedCommand);
    int currentCommandIndex = 0;
    for (int i = 0; i < splitedCommandCount; i++) {
      trim_command(splitedCommand[i]);
      // printf ("splitedCommand[%d] = %s\n", i, splitedCommand[i]);
      if(strstr(splitedCommand[i], "|") != NULL) pipeCount++;
    }

    // printf("pipeCount: %d\n", pipeCount);

    if(strcmp(splitedCommand[0], quit) == 0 || strcmp(splitedCommand[0], _exit) == 0) {
      return 0;
    }


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
        if(strcmp(splitedCommand[i], existBin[j]) == 0) {
          isInternalCommand = 1;
          break;
        }else isInternalCommand = 0;
      }
      // build the command
      // echo -e "previousCommandOutput" | _currentCommand
      char currentCommand[1024];
      strcpy(currentCommand, "echo -e \"");
      strcat(currentCommand, previousCommandOutput);
      strcat(currentCommand, "\" | ");
      if(isInternalCommand) strcat(currentCommand, "./bin/");
      while(i < splitedCommandCount && strstr(splitedCommand[i], "|") == NULL) {
        strcat(currentCommand, splitedCommand[i]);
        strcat(currentCommand, " ");
        i++;
      }
      // printf("currentCommand: %s\n", currentCommand);
      run_bash(currentCommand);
    }
    printf("%s\n", previousCommandOutput);
  }
}