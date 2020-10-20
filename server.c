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

int n, r;
int newsock;
int client_sock;

char ans[BLK];
char line[BLK];

int main() 
{ 
    int sfd, cfd, len; 
    struct sockaddr_in saddr, caddr; 
    int i, length;
    
    printf("1. create a socket\n");
    sfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sfd < 0) { 
        printf("socket creation failed\n"); 
        exit(0); 
    }
    
    printf("2. fill in server IP and port number\n");
    bzero(&saddr, sizeof(saddr)); 
    saddr.sin_family = AF_INET; 
    saddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    saddr.sin_port = htons(PORT);
    
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
 
       printf("server: accepted a client connection from\n");
       printf("-----------------------------------------------\n");
       //printf("    IP=%s  port=%d\n", inet_ntoa(caddr.sin_addr.s_addr),
       //                             ntohs(caddr.sin_port));
       printf("    IP=%s  port=%d\n", "127.0.0.1", ntohs(caddr.sin_port));
       printf("-----------------------------------------------\n");

       // Processing loop
       while(1){
         printf("server ready for next request ....\n");
         n = read(client_sock, line, MAX);
         if (n==0){
           printf("server: client died, server loops\n");
           close(cfd);
           break;
         }
         line[n]=0;
         // show the line string
         printf("server: read  n=%d bytes; line=[%s]\n", n, line);

        int index = 0;
        int spaceL = -1;
        char head[64];
        char tail[64];
        int sum;
        while (line[index] != NULL && index != -1) { // check if line is numbers
          printf("line: '%c', index: %d\n", line[index], index);
          if (line[index] == ' '/*strcmp(line[index], " ") == 0*/) { // is space
            printf("is space\n");
            if (spaceL != -1 || index == 0 || line[index + 1] == NULL) { // more than one space or first element (to prevent " 1234" and "1234 " from working)
              index = -2;
              printf("is repeat of space\n");
            }
            spaceL = index; // save location of space
          } else if (isdigit((int)line[index]) == 0) { // is not number
            printf("is not a number\n");
            index = -2;
          }
          printf("Made it through loop\n");
          ++index;
        }
        printf("index: %d\n", index);
        if (index != -1) { // line is fully numbers
          printf("Line is fully number\n");
          for (int i = 0; i < spaceL; i++) {
            head[i] = line[i];
          }
          head[spaceL] = NULL;
          int h = atoi(head);
          printf("Head: %s, %d\n", head, atoi(head));
          int j = spaceL + 1;
          for (; line[j] != NULL; j++) {
            tail[j - spaceL - 1] = line[j];
          }
          //tail[j-1] = NULL;
          printf("Tail: %s, %d\n", tail, atoi(tail));
          sum = atoi(head) + atoi(tail);
          printf("Sum: %d\n", sum);
          //strcpy(line, itoa(sum));
          sprintf(line, "%d", sum);
          printf("Line: %s\n", line);
        } else {
         strcat(line, " ECHO");
        }

         // send the echo line to client 
         n = write(client_sock, line, MAX);

         printf("server: wrote n=%d bytes; ECHO=[%s]\n", n, line);
         printf("server: ready for next request\n");
         return 0;
       }
    }
}


