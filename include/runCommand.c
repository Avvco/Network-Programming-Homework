#include <ctype.h>

#define READ_END 0
#define WRITE_END 1
int save_in;

/**
 * @brief run the command
 * There are three processes in this function which share 2 pipe.
 * The first will fork to a child process to run the command which read the stdin from second child and produce stdout to parent.
 * The second process is also the child process, that generate stdin for first child process.
 * The last process is the parent process, it will read the stdout from the first child process.
 * 
 * link1: write echo output to command stdin  (child2 -> child1)
 * link2: read command output to parent stdin (child1 -> parent)
 * 
 * @param command command to run
 */
int run_command(const char *command) {
  int save_in;
  save_in = dup(STDIN_FILENO);

  // copy previousCommandOutput to currentCommandOutput and clear previousCommandOutput
  char currentCommandOutput[PREVIOUS_COMMAND_OUTPUT_SIZE];
  memset(currentCommandOutput, 0, PREVIOUS_COMMAND_OUTPUT_SIZE);
  strcat(currentCommandOutput, previousCommandOutput);
  memset(previousCommandOutput, 0, PREVIOUS_COMMAND_OUTPUT_SIZE);

  int link1[2];
  int link2[2];
  pid_t pid1, pid2;
  char foo[4096 + 1];
  char cpcommand[MAX_COMMANDS_SIZE];

  char **_command = malloc(MAX_COMMANDS_SIZE * sizeof(char *));

  strcpy(cpcommand, command);
  command_parse(cpcommand, _command);


  if (pipe(link1) == -1)
    die("pipe1");
  if (pipe(link2) == -1)
    die("pipe2");

  if ((pid1 = fork()) == -1)
    die("fork1");

  if(pid1 == 0) {
    // child 1
    dup2(link1[READ_END], STDIN_FILENO);
    dup2(link2[WRITE_END], STDOUT_FILENO);
    close(link1[WRITE_END]);
    close(link2[READ_END]);
    execvp(_command[0], _command);
    char *error = malloc(128 * sizeof(char));
    strcpy(error, _command[0]);
    strcat(error, ": command not found");
    die(error);
  } else {
    if ((pid2 = fork()) == -1)
      die("fork2");
    if(pid2 == 0) {
      // child 2
      dup2(link1[WRITE_END], STDOUT_FILENO);
      close(link1[READ_END]);
      close(link2[READ_END]);
      close(link2[WRITE_END]);
      execl("./bin/echo", "-e", currentCommandOutput, (char *)0);
      die("echo: command not found");
    }else {
      // parent
      dup2(link2[READ_END], STDIN_FILENO);
      close(link1[READ_END]);
      close(link1[WRITE_END]);
      close(link2[WRITE_END]);
      int nbytes = 0;
      while(0 != (nbytes = read(link2[READ_END], foo, sizeof(foo)))) {
        //printf("foo: ");
        //printf("%.*s\n", nbytes, foo);
        memset(previousCommandOutput, 0, PREVIOUS_COMMAND_OUTPUT_SIZE);
        strcat(previousCommandOutput, foo);
        memset(foo, 0, 4096);
      }
      // remove the last line break
      if(strcmp(&previousCommandOutput[strlen(previousCommandOutput) - 1], "\n") == 0) previousCommandOutput[strlen(previousCommandOutput) - 1] = '\0';
      wait(NULL);
    }
    wait(NULL);
  }

  // return to the original stdin for parent process
  dup2(save_in, STDIN_FILENO);
}