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
#include <libgen.h>     // for dirname()/basename() asdf
#include <time.h> 

#define MAX 256
#define PORT 1234
#define BLK 1024

char gdir[MAX];    // gdir[ ] stores dir strings
char *dir[64];
int  ndir;

char gpath[MAX];   // gpath[ ] stores token strings
char *name[64];
int  ntoken;

//               0       1      2      3       4         5        6       7      8
char *cmd[] = {"lcat", "lls", "lcd", "lpwd", "lmkdir", "lrmdir", "lrm", "get", "put", 0};

struct sockaddr_in saddr; 
int sock, r, n;
char line[MAX], ans[MAX];

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

void getPathnames(char *env[]) {
    int index = 0;
    int i = 0;
    while (env[i]){
        printf("env[%d] = %s\n", i, env[i]);
        
        // Looking for PATH=
        if (strncmp(env[i], "PATH=", 5)==0){
        printf("show PATH: %s\n", env[i]);

        printf("decompose PATH into dir strings in gdir[ ]\n");
        strcpy(gdir, &env[i][5]);

        while(gdir[index] != '\0') { // decompose PATH into dir strings
            //printf("gdir[%d] == %c\n", index, gdir[index]);
            if (ndir == 0) {
                dir[ndir] = &gdir[index];
                ndir++;
            }
            if (gdir[index] == ':') {
                gdir[index] = '\0';
                dir[ndir] = &gdir[index + 1];
                ndir++;
            }
            index++;
        }
        printf("ndir == %d\n", ndir);
        
        break;
        }
        i++;
    } // end of large while loop
}

void decomposeLine() {
    int nindex = 1;
    if (line[0] != '\0'){ // get first element in name
        gpath[0] = line[0];
      	name[0] = &gpath[0];
      	printf("name[0] = %s\n", name[0]);
    }
    while(line[nindex] != '\0') { // go through pathname and make tokens
      	if (line[nindex] == ' ') {
            gpath[nindex] = '\0'; // start new token
            ntoken++;
            name[ntoken] = &gpath[nindex + 1]; // put new pointer in name
            printf("ntoken = %d\n", ntoken);
        } else {
            printf("to gpath = %c at %d\n", line[nindex], nindex);
            gpath[nindex] = line[nindex]; // input letters to gpath
   	    }
   	    nindex++;
    }
}

int main(int argc, char *argv[], char *env[]) 
{ 
    int n; char how[64];
    int i;
    char *$HOME = getenv("HOME");

    printf("1. create a socket\n");
    sock = socket(AF_INET, SOCK_STREAM, 0); 
    if (sock < 0) { 
        printf("socket creation failed\n"); 
        exit(0); 
    }
    
    printf("2. fill in server IP and port number\n");
    bzero(&saddr, sizeof(saddr)); 
    saddr.sin_family = AF_INET; 
    saddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    saddr.sin_port = htons(PORT); 
  
    printf("3. connect to server\n");
    if (connect(sock, (struct sockaddr *)&saddr, sizeof(saddr)) != 0) { 
        printf("connection with the server failed...\n"); 
        exit(0); 
    } 
    //n = write(sock, "TEST", MAX);

    // Get Pathnames
    //getPathnames(env);

    while (1){
     for (int i = 0; i < 128; i++) {
     	line[i] = '\0'; // clean line
     	gpath[i] = '\0'; // clean line
     	name[i] = NULL;
     }
     //gpath[0] = '\0';
     ntoken = 0;

      printf("input a line : ");
      fgets(line, MAX, stdin);
      line[strlen(line)-1] = 0;        // kill <CR> at end
      if (line[0]==0)                  // exit if NULL line
         exit(0);
      printf("line = %s\n", line);   // print line to see what you got
     //n = write(sock, line, MAX);
     // split line into usable strings
     decomposeLine();

    for (int i = 0; i < ntoken; i++) {
        printf("name[%d] = %s\n", i, name[i]);
    }

    if (name[0] != NULL) {
      int r = 0;
        switch (findCmd(name[0])) {
            case 0:
              printf("lcat\n");
              //cat(name[1]);
            break; // lcat
            case 1:
              printf("lls\n");
              //ls(name[1]);
            break; // lls
            case 2:
              printf("lcd\n");
              if (name[1] == NULL) {
                r = chdir($HOME);
              } else {
                r = chdir(name[1]);
              }
            break; // lcd
            case 3:
              printf("lpwd\n");
              r = getcwd(name[1], sizeof(name[1]));
              //pwd(name[1]);
            break; // lpwd
            case 4: 
              printf("lmkdir\n");
              r = mkdir(name[1], 0776);
            break; // lmkdir
            case 5:
              printf("lrmdir\n");
              r = rmdir(name[1]);
            break; // lrmdir
            case 6:
              printf("lrm\n");
              r = unlink(name[1]);
            break; // lrm
            case 7:
              printf("get\n");
              // Send ENTIRE line to server
              n = write(sock, line, MAX);
              printf("client: wrote n=%d bytes; line=(%s)\n", n, line);

              // Read a line from sock and show it
              n = read(sock, ans, MAX);
              printf("client: read  n=%d bytes; echo=(%s)\n",n, ans);
            break; // get
            case 8:
              printf("put\n");
              // Send ENTIRE line to server
              n = write(sock, line, MAX);
              printf("client: wrote n=%d bytes; line=(%s)\n", n, line);

              // Read a line from sock and show it
              n = read(sock, ans, MAX);
              printf("client: read  n=%d bytes; echo=(%s)\n",n, ans);
            break; // put
            default: // send to server for it to check
              printf("default\n");  
              // Send ENTIRE line to server
              n = write(sock, line, MAX);
              printf("client: wrote n=%d bytes; line=(%s)\n", n, line);

              // Read a line from sock and show it
              n = read(sock, ans, MAX);
              printf("client: read  n=%d bytes; echo=(%s)\n",n, ans);
        }
      }
    }
}
