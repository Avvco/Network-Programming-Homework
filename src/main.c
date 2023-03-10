#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dirent.h> 
#include <ctype.h>

#define die(e) do { fprintf(stderr, "%s\n", e); exit(EXIT_FAILURE); } while (0);

char existBin[100][100];
char existBinCount = 0;
char previousCommandOutput[32768 + 1];
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
    memset(previousCommandOutput, 0, 32768);
    while(0 != (nbytes = read(link[0], foo, sizeof(foo)))) {
      //printf("%.*s\n", nbytes, foo);
      strcat(previousCommandOutput, foo);
      memset(foo, 0, 4096);
    }
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
  memset(previousCommandOutput, 0, 32768);

  // mark this line if you want to use the system executables.
  setenv("PATH", "/bin:./bin", 1);
}

void compress_spaces(char *str) {
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

int main(int argc, char * argv[]) {
  char command[5000];
  char quit[] = "quit";
  char _exit[] = "exit";
  char _setenv[] = "setenv";
  char printenv[] = "printenv";
  char delim[] = " ";
  init();
  while (1) {
    char **res  = NULL;
    int n_spaces = 0;
    int isInternalCommand = 0;
    printf("%s", "% ");
    if(fgets(command, 5000 ,stdin) == NULL || strcmp(command, quit) == 10 || strcmp(command, _exit) == 10) { // 10 is the difference between the values of 'quit' and 'quit\n'
      break;
    }
    if(strcmp(command, "\n") == 0) continue;

    // remove all new line
    command[strcspn(command,"\n")] = '\0';
    compress_spaces(command);

    // cut the command with space
    char *p = strtok(command, delim);
    while(p) {
      res = realloc (res, sizeof (char*) * ++n_spaces);

      if(res == NULL)
        exit(-1); /* memory allocation failed */

      res[n_spaces-1] = p;

      p = strtok (NULL, " ");
    }

    /* realloc one extra element for the last NULL */
    res = realloc(res, sizeof (char*) * (n_spaces+1));
    res[n_spaces] = 0;

    for (int i = 0; i < n_spaces; i++)
      printf ("res[%d] = %s\n", i, res[i]);

    char _command[100];
    if(strcmp(res[0], printenv) == 0 || strcmp(res[0], printenv) == 10) {
      printf("printing env");
      printf("%s", getenv(res[1]));
    }else if(strcmp(res[0], _setenv) == 0) {
      printf("setting env");
      setenv(res[1], res[2], 1);
    }else {
      printf("running command");
      run_bash(command);
    }

    
    // check if it is an internal command

    /*for(int i = 0; i < existBinCount; i++) {
      // printf("bin %s %d\n",existBin[i],strcmp(command, existBin[i]));
      if(strcmp(command, existBin[i]) == 0 || strcmp(command, existBin[i]) == 10) {
        isInternalCommand = 1;
        break;
      }
    }

    if(isInternalCommand) {
      // printf("Internal command\n");
      char path[100];
      strcpy(path, "./bin/");
      strcat(path, command);
      run_bash(path);
    }else {
      // printf("External command\n");
      run_bash(command);
    }*/
    //printf("%s", previousCommandOutput);
    
  }
}