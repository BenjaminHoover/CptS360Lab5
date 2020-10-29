// CptS 360 - Lab 5 - server.c
// Base Code references from K.C Wang, and his textbook: Systems Programming in Unix/Linux
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

DIR *dir;       // dir stores DIR structures
char cwd[MAX];  // stores cwd
char virtualRoot[MAX]; // stores virtualroot as initial cwd
int  ndir;      // stores the number of directories

struct stat st;

//char gpath[MAX];   // gpath[ ] stores token strings
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
  strcpy(virtualRoot, cwd);
    
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
    
    char eot[MAX];
    n = read (client_sock, eot, MAX);
    // Processing loop
    while(1){
      int c = 0, size = 0;
      // Clean name
      for (int i = 0; i < MAX; i++) {
     	  name[i] = NULL;
      }
      for (int i = 0; i < BLK; i++) {
        line[i] = NULL;
      }

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
          
          //struct stat fstat;
          char fileLine1[MAX];
          int fd1, size, found = 0;
          fd1 = open(name[1], O_RDONLY);
          if (fd1 < 0) {
            printf("File not found"); 
            write(client_sock, &found, sizeof(found)); // write file not found
          } else { // file found and opened
            printf("inside else statement\n");
            found = 1; 
            write(client_sock, &found, sizeof(found)); // write file found
            stat(name[1], &st);
            int size = st.st_size;
            //snprintf(size, sizeof(fileLine1), "%zu", st.st_size);
            write(client_sock, &size, sizeof(size)); // send size

            read(client_sock, &found, sizeof(found)); // file created on client?
            printf("Confirmed client created file\n");
            if (found == 2) {
              while(n = read(fd1, fileLine1, MAX)){ // get info from server file
                write(client_sock, fileLine1, n); // send info
              }
            }
            close(fd1);
          }

          break;
          //n = write(client_sock, eot, MAX);
        case 1: // put
          c = 0;
          printf("put\n");

          char fileLine2[MAX];
          int fd2, fileSize = 0;
          fd2 = open(name[1], O_WRONLY | O_CREAT, 0644);
          if(fd2 < 0){ // attempt to open file
            printf("File creation failed.\n");
            //exit(1);
          } else { // file found/created
            int created = 1;
            write(client_sock, &created, sizeof(created)); // write creation successful
            read(client_sock, &fileSize, MAX); // get size
            //size = fileSize;
            printf("Size = %d, fd2 = %d\n", fileSize, fd2);
            while(c < fileSize) { // get data
              n = read(client_sock, fileLine2, MAX);
              c += n;
              write(fd2, fileLine2, n);
            }
          close(fd2);
          printf("Copy complete\n");
          }
          //n = write(client_sock, eot, MAX);
          break;
        case 2: // ls
          printf("ls\n");
          char temp[MAX];
          strcpy(temp, "\0");
          // if just ls, print cwd
          if(!name[1]){
            printf("Printing CWD...\n");
            n = write(client_sock, "Printing CWD...\n", MAX);
            dir = opendir(".");

            strcpy(temp, "\0");
            while ((entry = readdir(dir))){
              printf("%s ", entry->d_name);
              strcat(temp, entry->d_name);
              strcat(temp, " ");
            }
            n = write(client_sock, temp, MAX);
            strcpy(temp, "\0");
            closedir(dir);
          }
          // else, if name[1] is -l
          else if(strcmp(name[1], "-l") == 0) {
            strcpy(temp, "\0");
            if(!name[2]){
              printf("Printing CWD...\n");
              n = write(client_sock, "Printing CWD...\n", MAX);
              dir = opendir(".");

              while ((entry = readdir(dir))){
                getcwd(cwd, MAX);
                char *filepath = cwd;
                char ftime[64];
                char *t1 = "xwrxwrxwr-------"; 
                char *t2 = "----------------";
                char linkname[256];
                int linkSize = sizeof(linkname);
                struct stat fstat, *file_stats;
                file_stats = &fstat;
                strcat(filepath, "/");
                strcat(filepath, entry->d_name);

                if (!stat(filepath, &fstat)){
                  if (( file_stats-> st_mode & 0xF000) == 0x8000) { // if (S_ISREG()) 
                    printf("%c",'-'); 
                    strcat(temp, "-"); }
                  if (( file_stats-> st_mode & 0xF000) == 0x4000) { // if (S_ISDIR()) 
                    printf("%c",'d'); 
                    strcat(temp, "d"); }
                  if (( file_stats-> st_mode & 0xF000) == 0xA000) { // if (S_ISLNK()) 
                    printf("%c",'l');
                    strcat(temp, "l"); }
                  for (int i = 8; i >= 0; i--){
                    if (file_stats-> st_mode & (1 << i)) {// print r | w | x 
                      printf("%c", t1[i]);
                      strncat(temp, &t1[i], 1); 
                    } else {
                      printf("%c", t2[i]);
                      strncat(temp, &t2[i], 1); 
                    }
                  } 

                  char temp2[MAX];
                  strcat(temp, " ");
                  printf("%4ld ", file_stats-> st_nlink); // link count
                  sprintf(temp2, "%d" ,file_stats-> st_nlink);
                  strcat(temp, temp2); 
                  strcat(temp, " ");
                  printf("%4d ", file_stats-> st_gid); // gid 
                  sprintf(temp2, "%d" ,file_stats-> st_gid);
                  strcat(temp, temp2);
                  strcat(temp, " ");
                  printf("%4d ", file_stats-> st_uid); // uid 
                  sprintf(temp2, "%d" ,file_stats-> st_uid);
                  strcat(temp, temp2);
                  strcat(temp, " ");
                  printf("%8ld ", file_stats-> st_size); // file size 
                  sprintf(temp2, "%d" ,file_stats-> st_size);
                  strcat(temp, temp2);
                  strcat(temp, " ");

                  strcpy( ftime, ctime(& file_stats-> st_ctime) ); // print time in calendar form 
                  ftime[ strlen( ftime)-1] = 0; // kill \n at end 
                  printf("%s ", ftime); // print time
                  strcat(temp, ftime);
                  strcat(temp, " ");
                  printf("%s ", entry->d_name); // print name of file
                  strcat(temp,  entry->d_name);
                  strcat(temp, " ");

                  // print -> linkname if symbolic file 
                  if (( file_stats-> st_mode & 0xF000) == 0xA000){ 
                    // use readlink() to read linkname 
                    readlink(filepath ,linkname, linkSize);
                    printf("-> %s", linkname ); // print linked name
                    strcat(temp, "-> "); 
                    strcat(temp, linkname); 
                  }
                  printf("\n");
                }
                n = write(client_sock, temp, MAX);
                strcpy(temp, "\0");
              }
              closedir(dir);
            }
            else {
              strcpy(temp, "\0");
              printf("Printing given dir...");
              strcat(temp, "Printing given dir...");
              dir = opendir(name[2]);

              while ((entry = readdir(dir))){
                getcwd(cwd, MAX);
                char *filepath = cwd;
                char ftime[64];
                char *t1 = "xwrxwrxwr-------"; 
                char *t2 = "----------------";
                char linkname[256];
                int linkSize = sizeof(linkname);
                struct stat fstat, *file_stats;
                file_stats = &fstat;
                strcat(filepath, "/");
                strcat(filepath, entry->d_name);

                if (!stat(filepath, &fstat)){
                  if (( file_stats-> st_mode & 0xF000) == 0x8000) { // if (S_ISREG()) 
                    printf("%c",'-');
                    strcat(temp, "-"); }
                  if (( file_stats-> st_mode & 0xF000) == 0x4000) { // if (S_ISDIR()) 
                    printf("%c",'d');
                    strcat(temp, "d"); }
                  if (( file_stats-> st_mode & 0xF000) == 0xA000) { // if (S_ISLNK()) 
                    printf("%c",'l');
                    strcat(temp, "l"); }

                  for (int i = 8; i >= 0; i--){
                    if (file_stats-> st_mode & (1 << i)) {// print r | w | x 
                      printf("%c", t1[i]);
                      strncat(temp, &t1[i], 1); 
                    } else { 
                      printf("%c", t2[i]);
                      strncat(temp, &t2[i], 1); 
                    }
                  } 

                  char temp2[MAX];
                  strcat(temp, " ");
                  printf("%4ld ", file_stats-> st_nlink); // link count
                  sprintf(temp2, "%d" ,file_stats-> st_nlink);
                  strcat(temp, temp2); 
                  printf("%4d ", file_stats-> st_gid); // gid
                  sprintf(temp2, "%d" ,file_stats-> st_gid);
                  strcat(temp, temp2);
                  printf("%4d ", file_stats-> st_uid); // uid
                  sprintf(temp2, "%d" ,file_stats-> st_uid);
                  strcat(temp, temp2); 
                  printf("%8ld ", file_stats-> st_size); // file size
                  sprintf(temp2, "%d" ,file_stats-> st_size);
                  strcat(temp, temp2); 

                  strcpy( ftime, ctime(& file_stats-> st_ctime) ); // print time in calendar form 
                  ftime[ strlen( ftime)-1] = 0; // kill \n at end 
                  printf("%s ", ftime); // print time
                  strcat(temp, ftime);

                  printf("%s ", entry->d_name); // print name of file
                  strcat(temp, entry->d_name);

                  // print -> linkname if symbolic file 
                  if (( file_stats-> st_mode & 0xF000) == 0xA000){ 
                    // use readlink() to read linkname 
                    readlink(filepath ,linkname, linkSize);
                    printf("-> %s", linkname ); // print linked name
                    strcat(temp, "-> ");
                    strcat(temp, linkname); 
                  }
                  printf("\n");
                }
                n = write(client_sock, temp, MAX);
                strcpy(temp, "\0");
              }
              closedir(dir);
            }
          }
          //else, print directory in name[1]
          else {
            strcpy(temp, "\0");
            printf("Printing given dir...");
            strcat(temp, "Printing given dir...");
            dir = opendir(name[1]);

            while ((entry = readdir(dir))){
              printf("%s ", entry->d_name);
              strcat(temp, entry->d_name);
              strcat(temp, " ");
            }
            n = write(client_sock, temp, MAX);
            closedir(dir);
          }
          printf("\n");
          n = write(client_sock, eot, MAX);
          break;
        case 3: // cd
          printf("cd\n");

          getcwd(cwd, MAX);
          // if argument name[1] is not empty
          if (name[1]){
            if(((strcmp(name[1], "..") == 0) || (strcmp(name[1], "../") == 0)) && (strcmp(cwd, virtualRoot) == 0)){
              chdir(virtualRoot);
              getcwd(cwd, MAX);
              printf("cwd after cd = %s\n", cwd);
            }
            else if (chdir(name[1]) == 0){
              getcwd(cwd, MAX);
              printf("cwd after cd = %s\n", cwd);
            }
            else{
              printf("Invalid directory.\n");
            }
          }
          else{
            chdir(virtualRoot);
            getcwd(cwd, MAX);
            printf("cwd after cd = %s\n", cwd);
          }
          n = write(client_sock, eot, MAX);
          break;
        case 4: // pwd
          printf("pwd\n");

          getcwd(cwd, MAX);
          printf("cwd = %s\n", cwd);
          n = write(client_sock, cwd, MAX);
          n = write(client_sock, eot, MAX);
          break;
        case 5: // mkdir
          printf("mkdir\n");

          mkdir(name[1], 0755);
          n = write(client_sock, eot, MAX);
          break;
        case 6: // rmdir
          printf("rmdir\n");

          rmdir(name[1]);
          n = write(client_sock, eot, MAX);
          break;
        case 7: // rm
          printf("rm\n");
          
          unlink(name[1]);
          n = write(client_sock, eot, MAX);
          break;
        case 8: // exit
          n = write(client_sock, eot, MAX);
          return 0;
          break;
        default: // do whatever
          printf("default\n");
          n = write(client_sock, eot, MAX);
        } // end of switch statement
      } // end of if statement

      // send the echo line to client 
      //n = write(client_sock, line, MAX);
      //n = write(client_sock, eot, MAX);

      printf("server: wrote n=%d bytes; ECHO=[%s]\n", n, line);
      printf("server: ready for next request\n");
    } // end of processing loop
  } // end of connection loop
  printf("Why are you here?\n");
}
