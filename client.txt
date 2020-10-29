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

//#include "myls.c"

#define MAX 256
#define PORT 1234
#define BLK 1024
#define BLKSIZE 4096

DIR *dir;       // dir stores DIR structures
char cwd[MAX];  // stores cwd
int  ndir;

char gpath[MAX];   // gpath[ ] stores token strings
char *name[64];
int  ntoken;

//               0       1      2      3       4         5        6       7      8
char *cmd[] = {"lcat", "lls", "lcd", "lpwd", "lmkdir", "lrmdir", "lrm", "get", "put", 0};

struct sockaddr_in saddr; 
int sock, r, n;
char line[MAX], ans[MAX];

struct stat st;

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
    /*int index = 0;
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
    } // end of large while loop*/
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

int mycat() {
  int fd, i, m, n;
  char buf[BLKSIZE];
  //for (int i = 0; i < ntoken; i++) {
          if (name[i+2] == NULL) { // default
            fd = 0;
            if (name[1]) {
              fd = open(name[i + 1], O_RDONLY);
              if (fd < 0) { return 0; }
            }
            while (n = read (fd, buf, BLKSIZE)) {
              m = write (1, buf, n);
            }
          } else if (name[i+1] != NULL && strcmp(name[i+2], ">") == 0 && name[i+3] != NULL) { //copy file content
            int gd, n, total = 0;
            //if (name[i+3] == NULL) { return 0; }
            if (((fd = (open(name[i+1], O_RDONLY))) < 0)) { printf("File not found\n"); return 0; }
            if (((gd = open(name[i+3], O_WRONLY|O_CREAT, 0644)) < 0) || (gd == fd)) {
              printf("File not found\n");
              return 0;
            }
            printf("Files opened\n");
            while (n = read(fd, buf, BLKSIZE)) {
              write(gd, buf, n);
              total += n;
            }
            printf("total bytes copied = %d\n", total);
            close(fd); close(gd);
          }
		//} // end of pipeloop*/
    return 1;
}

int main(int argc, char *argv[], char *env[]) 
{ 
    int n; char how[64];
    char eot[MAX];
    struct dirent *entry;
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

    strcpy(eot, "ENDOFTRANSFER");
    strcat(eot, $HOME);
    strcat(eot, "ENDOFTRANSFER");
    //printf("eot: %s\n", eot);
    n = write(sock, eot, MAX);

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
              mycat();
              //printf("endCat\n");
              //cat(name[1]);
            break; // lcat
            case 1:
              printf("lls\n");
              // if just ls, print cwd
              if(!name[1]){
                printf("Printing CWD...\n");
                dir = opendir(".");

                while ((entry = readdir(dir))){
                  printf("%s ", entry->d_name);
                }
                closedir(dir);
              }
              // else, if name[1] is -l
              else if(strcmp(name[1], "-l") == 0) {
                
                if(!name[2]){
                  printf("Printing CWD...\n");
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
                      if (( file_stats-> st_mode & 0xF000) == 0x8000) // if (S_ISREG()) 
                        printf("%c",'-'); 
                      if (( file_stats-> st_mode & 0xF000) == 0x4000) // if (S_ISDIR()) 
                        printf("%c",'d'); 
                      if (( file_stats-> st_mode & 0xF000) == 0xA000) // if (S_ISLNK()) 
                        printf("%c",'l'); 

                      for (int i = 8; i >= 0; i--){
                        if (file_stats-> st_mode & (1 << i)) // print r | w | x 
                          printf("%c", t1[ i]); 
                        else 
                          printf("%c", t2[ i]); 
                      } 

                      printf("%4ld ", file_stats-> st_nlink); // link count 
                      printf("%4d ", file_stats-> st_gid); // gid 
                      printf("%4d ", file_stats-> st_uid); // uid 
                      printf("%8ld ", file_stats-> st_size); // file size 

                      strcpy( ftime, ctime(& file_stats-> st_ctime) ); // print time in calendar form 
                      ftime[ strlen( ftime)-1] = 0; // kill \n at end 
                      printf("%s ", ftime); // print time

                      printf("%s ", entry->d_name); // print name of file

                      // print -> linkname if symbolic file 
                      if (( file_stats-> st_mode & 0xF000) == 0xA000){ 
                        // use readlink() to read linkname 
                        readlink(filepath ,linkname, linkSize);
                        printf("-> %s", linkname ); // print linked name 
                      }
                      printf("\n");
                    }
                  }
                  closedir(dir);
                }
                else {
                  printf("Printing given dir...");
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
                      if (( file_stats-> st_mode & 0xF000) == 0x8000) // if (S_ISREG()) 
                        printf("%c",'-'); 
                      if (( file_stats-> st_mode & 0xF000) == 0x4000) // if (S_ISDIR()) 
                        printf("%c",'d'); 
                      if (( file_stats-> st_mode & 0xF000) == 0xA000) // if (S_ISLNK()) 
                        printf("%c",'l'); 

                      for (int i = 8; i >= 0; i--){
                        if (file_stats-> st_mode & (1 << i)) // print r | w | x 
                          printf("%c", t1[ i]); 
                        else 
                          printf("%c", t2[ i]); 
                      } 

                      printf("%4ld ", file_stats-> st_nlink); // link count 
                      printf("%4d ", file_stats-> st_gid); // gid 
                      printf("%4d ", file_stats-> st_uid); // uid 
                      printf("%8ld ", file_stats-> st_size); // file size 

                      strcpy( ftime, ctime(& file_stats-> st_ctime) ); // print time in calendar form 
                      ftime[ strlen( ftime)-1] = 0; // kill \n at end 
                      printf("%s ", ftime); // print time

                      printf("%s ", entry->d_name); // print name of file

                      // print -> linkname if symbolic file 
                      if (( file_stats-> st_mode & 0xF000) == 0xA000){ 
                        // use readlink() to read linkname 
                        readlink(filepath ,linkname, linkSize);
                        printf("-> %s", linkname ); // print linked name 
                      }
                      printf("\n");
                    }
                  }
                  closedir(dir);
                }
              }
              //else, print directory in name[1]
              else {
                printf("Printing given dir...");
                dir = opendir(name[1]);

                while ((entry = readdir(dir))){
                  printf("%s ", entry->d_name);
                }
                closedir(dir);
              }
              printf("\n");
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
              char buf[64];
              r = getcwd(buf, sizeof(buf));
              printf("%s\n", buf);
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
              printf("client: wrote n=%d bytes; line=(%s)\n", n, line); // send request to server

              int fileSize = 0, bytesRead = 0, gd, br;
              char cbuf[BLKSIZE];
              // Read a line from sock
              n = read(sock, fileSize, MAX);
              printf("Filesize = %s \n", fileSize);
              if ((gd = open(name[1], O_WRONLY|O_CREAT, 0644)) < 0) {
                printf("File not opened\n");
              } else { // begin copying
                while (fileSize > bytesRead) {// read whole file
                  n = read(sock, cbuf, MAX);
                  write(gd, cbuf, br);
                  bytesRead += br;
                }
                close (gd);
                printf("Copy complete\n");
              }
            break; // get
            case 8:
              printf("put\n");
              fileSize = 0; 
              int fd;
              char sbuf[BLKSIZE];
              // Send ENTIRE line to server
              if (((fd = (open(name[1], O_RDONLY))) < 0)) { 
                printf("File not found\n"); 
              } else { // begin transfer
                n = write(sock, line, MAX); // send name and command
                stat(name[1], &st);
                n = write(sock, st.st_size, MAX); // send size
                printf("client: put %s of %d bytes\n", name[1], st.st_size);
                
                while (n = read(fd, sbuf, BLKSIZE)) {
                  n = write(sock, sbuf, BLKSIZE); // send size
                }
                close(fd);
                printf("Transfer Complete\n");
              }
            break; // put
            default: // send to server for it to check
              printf("default\n");  
              // Send ENTIRE line to server
              n = write(sock, line, MAX);
              printf("client: wrote n=%d bytes; line=(%s)\n", n, line);

              // Read output from server
              printf("Server return:\n");
              do {
                n = read(sock, ans, MAX);
                if (strcmp(ans, eot) != 0) {
                  printf("%s\n", ans);
                }
              } while (strcmp(ans, eot) != 0);
        }
      }
    }
}

