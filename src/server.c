#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <dirent.h> 
#include <ctype.h>


#define DEFAULT_PORT    64550
#define BUFF_SIZE       20480
#define SELECT_TIMEOUT  5
#define MAX_CLIENTS     10

#include "../include/session.c"
#include "../include/dbService.c"
#include "../include/commandParse.c"
#include "./shell.c"


/*int main() {
  char a[] = "as";
  char b[] = "as";
  login(a,b);
}*/

void init() {
  initShell();
  // init session
  session = malloc(MAX_CLIENTS * sizeof(Session));
  for(int i = 0 ; i < MAX_CLIENTS ; i++) { 
    session[i].id = -1; 
    session[i].assignedId = -1;
    // session[i].loggedInUserId = malloc(2048 * sizeof(char));
    session[i].pipeCounter = 0;
    session[i].port = 0;
    session[i].ip = malloc(16 * sizeof(char));
    session[i].name = malloc(256 * sizeof(char));
    session[i].savedCommandOutput = malloc(PREVIOUS_COMMAND_OUTPUT_SIZE * sizeof(char));
    session[i].previousCommandOutput = malloc(PREVIOUS_COMMAND_OUTPUT_SIZE * sizeof(char));
    session[i].providedUsername = malloc(1024 * sizeof(char));
    session[i].providedPassword = malloc(1024 * sizeof(char));
    session[i].mode = -1;
    session[i].inputMode = -1;
  }
}


// set sock to non-blocking mode
void setSockNonBlock(int sock) {
  int flags;
  flags = fcntl(sock, F_GETFL, 0);
  if (flags < 0) {
    perror("fcntl(F_GETFL) failed");
    exit(EXIT_FAILURE);
  }
  if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0) {
    perror("fcntl(F_SETFL) failed");
    exit(EXIT_FAILURE);
  }
}

// update maxfd
int updateMaxfd(fd_set fds, int maxfd) {
  int i;
  int new_maxfd = 0;
  for (i = 0; i <= maxfd; i++) {
    if (FD_ISSET(i, &fds) && i > new_maxfd) {
      new_maxfd = i;
    }
  }
  return new_maxfd;
}

int main(int argc, char **argv, char **envp) {
  fprintf(stderr, "server started\n");
  init();

  unsigned short int port;
  // argument to custom port
  if (argc == 2) {
    port = atoi(argv[1]);
  } else if (argc < 2) {
    port = DEFAULT_PORT;
  } else {
    fprintf(stderr, "USAGE: %s [port]\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  // create socket
  int sock;
  if ( (sock = socket(PF_INET, SOCK_STREAM, 0)) == -1 ) {
    perror("socket failed, ");
    exit(EXIT_FAILURE);
  }
  // printf("socket done\n");

  //in case of 'address already in use' error message
  int yes = 1;
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))) {
    perror("setsockopt failed");
    exit(EXIT_FAILURE);
  }

  setSockNonBlock(sock);

  // create binding socket address
  struct sockaddr_in bind_addr;
  memset(&bind_addr, 0, sizeof(bind_addr));
  bind_addr.sin_family = AF_INET;
  bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);  // accept address
  bind_addr.sin_port = htons(port);               // migrate host byte order to network byte order

  //bind sock to socket address
  if ( bind(sock, (struct sockaddr *) &bind_addr, sizeof(bind_addr)) == -1 ) {
    perror("bind failed, ");
    exit(EXIT_FAILURE);
  }
  // printf("bind done\n");

  if (listen(sock, MAX_CLIENTS) == -1) {
    perror("listen failed.");
    exit(EXIT_FAILURE);
  }
  // printf("listen done\n");

  // initialize parameter for select, only for read from client
  fd_set readfds;
  fd_set readfds_bak; // backup for readfds
  struct timeval timeout;
  int maxfd;
  maxfd = sock;
  FD_ZERO(&readfds);
  FD_ZERO(&readfds_bak);
  FD_SET(sock, &readfds_bak);

  // looping accepting client request
  int new_sock;
  struct sockaddr_in client_addr;
  socklen_t client_addr_len;
  char client_ip_str[INET_ADDRSTRLEN];
  int res;
  int i;
  char buffer[BUFF_SIZE];
  int recv_size;

  while (1) {
    // reset readfds and timeout
    readfds = readfds_bak;
    maxfd = updateMaxfd(readfds, maxfd);
    timeout.tv_sec = SELECT_TIMEOUT;
    timeout.tv_usec = 0;
    //fprintf(stderr, "selecting maxfd=%d\n", maxfd);

    // select, not setting writefds errorfds here
    res = select(maxfd + 1, &readfds, NULL, NULL, &timeout);
    if (res == -1) {
      perror("select failed");
      exit(EXIT_FAILURE);
    } else if (res == 0) {
      //fprintf(stderr, "no socket ready for read within %d secs\n", SELECT_TIMEOUT);
      continue;
    }

    // check every socket and read if accept
    for (i = 0; i <= maxfd; i++) {
      if (!FD_ISSET(i, &readfds)) {
        continue;
      }
      // socket avaliable
      if ( i == sock ) {
        // server socket
        client_addr_len = sizeof(client_addr);
        new_sock = accept(sock, (struct sockaddr *) &client_addr, &client_addr_len);
        if (new_sock == -1) {
          perror("accept failed");
          exit(EXIT_FAILURE);
        }
        if (!inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip_str, sizeof(client_ip_str))) {
          perror("inet_ntop failed");
          exit(EXIT_FAILURE);
        }
        fprintf(stderr, "accept a client from: %s:%d\n", client_ip_str, ntohs(client_addr.sin_port));

        // set new_sock to non-blocking
        setSockNonBlock(new_sock);
        // add new_sock into select
        if(new_sock > maxfd) {
          maxfd = new_sock;
        }
        FD_SET(new_sock, &readfds_bak);
        char *buffer = malloc(sizeof(char) * 256);
        strcpy(buffer, prompt);
        buffer = strcat(buffer, prefix);

        int sessionFound = 0;
        for(int _session = 0; _session < MAX_CLIENTS; _session++) {
          if(session[_session].id == new_sock) {
            sessionFound = 1;
            break;
          }
        }
        if(sessionFound == 0) {
          session[new_sock].id = new_sock;
          session[new_sock].pipeCounter = 0;
          session[new_sock].port = ntohs(client_addr.sin_port);
          session[new_sock].inputMode = -1;
          session[new_sock].mode = -1;
          strcpy(session[new_sock].ip, client_ip_str);
          // strcpy(session[new_sock].name, "anonymous");
          memset(session[new_sock].savedCommandOutput, 0, sizeof(session[new_sock].savedCommandOutput));
          for(int _assignedId = 0 ; _assignedId < MAX_CLIENTS; _assignedId++) {
            int used = 0;
            for(int j = 0; j < MAX_CLIENTS; j++) {
              if(session[j].assignedId == _assignedId) {
                used = 1;
                break;
              }
            }
            if(used == 0) {
              session[new_sock].assignedId = _assignedId;
              break;
            }
          }
        }

        send(new_sock, buffer, strlen(buffer) + 1, 0);
      } else {
        // client socket, eligible to read from client
        memset(buffer, 0, sizeof(buffer));
        if((recv_size = recv(i, buffer, sizeof(buffer), 0)) == -1) {
          perror("recv failed");
          exit(EXIT_FAILURE);
        }
        // fprintf(stderr, "recved from new_sock=%d : %s(%d length string)\n", i, buffer, recv_size);
        fprintf(stderr, "recved from new_sock=%d : %s", i, buffer);

        memset(session[i].previousCommandOutput, 0, sizeof(session[i].previousCommandOutput));
        trim_command(buffer);
        // restart point for mode
      restart:
        if(session[i].mode == 2 || strcmp(buffer, quit) == 0 || strcmp(buffer, _exitTerm) == 0) { // logged in or exit handling
          int status;
          if(strcmp(buffer, quit) == 0 || strcmp(buffer, _exitTerm) == 0) {
            status = -1;
          }else {
            status = process(buffer, &session[i], envp);
          }
          if(status == -1) {
            fprintf(stderr, "client request to exit from new_sock=%d\n", i);
            if(close(i) == -1) {
              perror("close failed");
              exit(EXIT_FAILURE);
            }
            fprintf(stderr, "close new_sock=%d done\n", i);

            // remove socket from select, disconnect with client
            FD_CLR(i, &readfds_bak);

            // clear session
            for(int _session = 0; _session < MAX_CLIENTS; _session++) {
              if(session[_session].id == i) {
                session[_session].id = -1;
                session[_session].assignedId = -1;
                session[_session].inputMode = -1;
                session[_session].loggedInUserId = NULL;
                break;
              }
            }
            continue;
          }else if(status == 0) {
            sprintf(session[i].previousCommandOutput, "%s\n(%s)%s", session[i].previousCommandOutput, getNameByUserId(session[i].loggedInUserId), prefix);
          }else if(status == 1) {
            sprintf(session[i].previousCommandOutput, "%s(%s)%s", session[i].previousCommandOutput, getNameByUserId(session[i].loggedInUserId), prefix);
          }
          
          // send result
          if (send(i, session[i].previousCommandOutput, strlen(session[i].previousCommandOutput) + 1, 0) == -1) {
            perror("send failed");
            exit(EXIT_FAILURE);
          }
          fprintf(stderr, "send to new_sock=%d done with length %d\n", i, strlen(session[i].previousCommandOutput) + 1);
        }else if(session[i].mode == -1) {
          if(strcmp(buffer, "0") == 0) { // next mode login
            session[i].mode = 0;
            goto restart;
          }else if(strcmp(buffer, "1") == 0) { // next mode register
            session[i].mode = 1;
            goto restart;
          }else {
            char *buffer = malloc(sizeof(char) * 64);
            strcpy(buffer, prompt);
            buffer = strcat(buffer, prefix);
            send(i, buffer, strlen(buffer) + 1, 0);
          }
        }else if(session[i].mode == 0) { // mode login
          if(session[i].inputMode == -1) {
            session[i].inputMode = 0;
            char *sent = malloc(sizeof(char) * 128);
            strcpy(sent, "Username: ");
            send(i, sent, strlen(sent) + 1, 0);
          }else if(session[i].inputMode == 0) {
            strcpy(session[i].providedUsername, buffer);
            session[i].inputMode = 1;
            char *sent = malloc(sizeof(char) * 128);
            strcpy(sent, "Password: ");
            send(i, sent, strlen(sent) + 1, 0);
          }else if(session[i].inputMode == 1) {
            strcpy(session[i].providedPassword, buffer);
            int status = login(&session[i]);
            if(status == 0) {
              char *sent = malloc(sizeof(char) * 128);
              snprintf(sent, 128, "Username or password incorrect.\n");
              send(i, sent, strlen(sent) + 1, 0);
            }
            session[i].mode = -2;
            goto restart;
          }
        }else if(session[i].mode == 1) { // mode register
          if(session[i].inputMode == -1) {
            session[i].inputMode = 0;
            char *sent = malloc(sizeof(char) * 128);
            strcpy(sent, "Username: ");
            send(i, sent, strlen(sent) + 1, 0);
          }else if(session[i].inputMode == 0) {
            strcpy(session[i].providedUsername, buffer);
            session[i].inputMode = 1;
            char *sent = malloc(sizeof(char) * 128);
            strcpy(sent, "Password: ");
            send(i, sent, strlen(sent) + 1, 0);
          }else if(session[i].inputMode == 1) {
            strcpy(session[i].providedPassword, buffer);
            int status = registerUser(session[i].providedUsername, session[i].providedPassword);
            session[i].mode = -2;

            char *sent = malloc(sizeof(char) * 1280);
            snprintf(sent, 1280, "Register user %s %s\n", session[i].providedUsername, status == 0 ? "success" : "failed");
            send(i, sent, strlen(sent) + 1, 0);
            goto restart;
          }
        }else if(session[i].mode == -2) { // reset mode
          session[i].inputMode = -1;
          session[i].mode = -1;
          char *sent = malloc(sizeof(char) * 512);
          if(session[i].loggedInUserId) {
            fprintf(stderr, "user %s logged in\n", session[i].name);
            snprintf(sent, 512, "Welcome %s!\n(%s)$ ", session[i].name, session[i].name);
            session[i].mode = 2;
          }else {
            snprintf(sent, 512, "%s%s ", prompt, prefix);
          }
          send(i, sent, strlen(sent) + 1, 0);
        }
      }
    }
  }
  return 0;
}