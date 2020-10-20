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

struct sockaddr_in saddr; 
int sock, r, n;
char line[MAX], ans[MAX];

int main(int argc, char *argv[], char *env[]) 
{ 
    int n; char how[64];
    int i;

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
    
    while (1){
      printf("input a line : ");
      fgets(line, MAX, stdin);
      line[strlen(line)-1] = 0;        // kill <CR> at end
      if (line[0]==0)                  // exit if NULL line
         exit(0);

    // Looking for PATH=
    if (strncmp(env[i], "PATH=", 5)==0){
      printf("show PATH: %s\n", env[i]);

      printf("decompose PATH into dir strings in gdir[ ]\n");
      strcpy(gdir, &env[i][5]);

      /*************** 1 ******************************
      Write YOUR code here to decompose PATH into dir strings in gdir[ ]
      pointed by dir[0], dir[1],..., dir[ndir-1]
      ndir = number of dir strings
      print dir strings
      ************************************************/     		
   int index = 0;
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

     /***************** 2 **********************
      Write YOUR code here to decompose line into token strings in gpath[ ]
      pointed by name[0], name[1],..., name[ntoken-1]
      ntoken = number of token strings
      print the token strings
      ************************************************/    
    int nindex = 1;
      if (line[0] != '\0'){ // get first element in name
        gpath[0] = line[0];
      	name[0] = &gpath[0];
      	//printf("name[0] = %s\n", name[0]);
      }
      while(line[nindex] != '\0') { // go through pathname and make tokens
      	if (line[nindex] == ' ') {
   		gpath[nindex] = '\0'; // start new token
		ntoken++;
   		name[ntoken] = &gpath[nindex + 1]; // put new pointer in name
   		//printf("ntoken = %d\n", ntoken);
   	} else {
   		//printf("to gpath = %c at %d\n", line[index], index);
   		gpath[nindex] = line[nindex]; // input letters to gpath
   	}
   	nindex++;
    }

      // Send ENTIRE line to server
      n = write(sock, line, MAX);
      printf("client: wrote n=%d bytes; line=(%s)\n", n, line);

      // Read a line from sock and show it
      n = read(sock, ans, MAX);
      printf("client: read  n=%d bytes; echo=(%s)\n",n, ans);
    }
}

