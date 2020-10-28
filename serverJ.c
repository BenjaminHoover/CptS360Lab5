// CptS 360 - Lab 5 - server.c
// Contributers: Jonathan Coronado, Benjamin Hoover
// About lab: This lab handles filesystem commands from client to server
// About file: This file in particular handles the server side.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <arpa/inet.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <libgen.h>     // for dirname()/basename()
#include <time.h> 

#include <ctype.h>

#define MAX 256
#define PORT 1234 
#define BLK 1024

DIR *dir;    // dir stores DIR structures
char cwd[MAX]; // stores cwd
int  ndir;

char gpath[MAX];   // gpath[ ] stores token strings
char *name[64];
int  ntoken;

//               0      1      2     3     4      5        6       7      8  
char *cmd[] = {"get", "put", "ls", "cd", "pwd", "mkdir", "rmdir", "rm", "exit", 0};

int n, r;
int newsock;
int client_sock;

char ans[BLK];
char line[BLK];

// Method to find command based on input
int findCmd(char *command)
{
   int i = 0;
   while(cmd[i]){
     if (strcmp(command, cmd[i])==0)
         return i;
     i++;
   }
   return -1;
}

// Method to decompose Line into tokens
int decomposeLine()
{
    char *s;
    char *l;
    int i = 0;
    l = line;

    while (s = strtok_r(l, " ", &l))
    {
        if ((strcmp(s, "\n")==0) || (strcmp(s, "")==0) || (strcmp(s, " ")==0))
        {
            name[i] = NULL;
            break;
        }
        name[i] = s;
        i++;
    }
    return i;
}

// Main method
int main(int argc, char *argv[], char *env[]) 
{
  int sfd, cfd, len; 
  struct sockaddr_in saddr, caddr; 
  int i, length;
  struct dirent *entry;

  // get cwd
  getcwd(cwd, MAX);
    
  // Create Socket
  printf("1. create a socket\n");
  sfd = socket(AF_INET, SOCK_STREAM, 0); 
  if (sfd < 0) { 
    printf("socket creation failed\n"); 
    exit(0); 
  }
    
  // Initialize server IP and Port
  printf("2. fill in server IP and port number\n");
  bzero(&saddr, sizeof(saddr)); 
  saddr.sin_family = AF_INET; 
  saddr.sin_addr.s_addr = htonl(INADDR_ANY); 
  saddr.sin_port = htons(PORT);
    
  // Bind socket to server
  printf("3. bind socket to server\n");
  if ((bind(sfd, (struct sockaddr *)&saddr, sizeof(saddr))) != 0) { 
    printf("socket bind failed\n"); 
    exit(0); 
  }
      
  // Now server is ready to listen and verification 
  if ((listen(sfd, 5)) != 0) { 
    printf("Listen failed\n"); 
    exit(0); 
  }
   
  while(1){
    // Try to accept a client connection as descriptor newsock
    printf("server: try to accept a new connection\n");
    length = sizeof(caddr);
    client_sock = accept(sfd, (struct sockaddr *)&caddr, &length);
    if (client_sock < 0){
      printf("server: accept error\n");
      exit(1);
    }

    // print successfull connection
    printf("server: accepted a client connection from\n");
    printf("-----------------------------------------------\n");
    printf("    IP=%s  port=%d\n", "127.0.0.1", ntohs(caddr.sin_port));
    printf("-----------------------------------------------\n");

    // Processing loop
    while(1){
      // Read line from client_sock into line
      printf("server ready for next request ....\n");
      n = read(client_sock, line, MAX);
      printf("line = %s\n", line);

      // failed read
      if (n==0){
        printf("server: client died, server loops\n");
        close(cfd);
        break;
      }

      // display line info
      printf("line = %s\n", line);
      line[n]=0;
      printf("line = %s\n", line);
      printf("server: read  n=%d bytes; line=[%s]\n", n, line);

      // decompose line
      decomposeLine();

      // switch-case for commands
      if (name[0] != NULL) {
        switch (findCmd(name[0])) {
        case 0: // get
          printf("get\n");
          break; 
        case 1: // put
          printf("put\n");
          break;
        case 2: // ls
          printf("ls\n");

          // if just ls, print cwd
          if(!name[1]){
            printf("Printing CWD...\n");
            dir = opendir(".");

            while ((entry = readdir(dir))){
              printf("%s ", entry->d_name);
            }
            closedir(dir);
          }
          // else, print input dir after the ls
          else{
            printf("Printing given dir...");
            dir = opendir(name[1]);

            while ((entry = readdir(dir))){
              printf("%s ", entry->d_name);
            }
            closedir(dir);
          }
          printf("\n");

          break;
        case 3: // cd
          printf("cd\n");
          
          // if argument name[1] is not empty
          if (strcmp(name[1], "")){
            chdir(name[1]);
            getcwd(cwd, MAX);
            printf("cwd = %s\n", cwd);
          }
          else{
            printf("Invalid directory.\n");
          }
          break;
        case 4: // pwd
          printf("pwd\n");

          getcwd(cwd, MAX);
          printf("cwd = %s\n", cwd);
          break;
        case 5: // mkdir
          printf("mkdir\n");

          mkdir(name[1], 0755);
          break;
        case 6: // rmdir
          printf("rmdir\n");

          rmdir(name[1]);
          break;
        case 7: // rm
          printf("rm\n");
          
          unlink(name[1]);
          break;
        case 8: // exit
          return 0;
          break;
        default: // do whatever
          printf("default\n");
        } // end of switch statement
      } // end of if statement

      // send the echo line to client 
      n = write(client_sock, line, MAX);

      printf("server: wrote n=%d bytes; ECHO=[%s]\n", n, line);
      printf("server: ready for next request\n");
    } // end of processing loop
  } // end of connection loop
  printf("Why are you here?\n");
}
