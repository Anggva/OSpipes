#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <signal.h>
typedef int bool;
#define true 1
#define false 0
#define MAXLINE 80
// Table structure
typedef struct T{
   char id[50]; 
   int value; 
} Table;
typedef struct S{
  int origin;
  pid_t p;
  char op;
  char id[50];
  int value;
  int result;
} SendStruct;


int initSize(FILE*);
int tableUpdate (char*, int, int);
int tableRead (char*, int, int);
bool checkDone (int);
Table TAB[100];
int globalPosition = 0;

int main(int argc, char *argv[])
{
  int p;
  FILE* transfile;
  FILE* trans1;
  FILE* trans2;
  FILE* init;
  FILE* logfile;
  int sPipe[1][2];
  int rPipe[2][2];
  int pids[3];
  int n;
  logfile = fopen(argv[2], "w");
  fclose(logfile);
  // Pipe declaration
  p = pipe(sPipe[0]);
  if (p < 0) {
    printf("pipe error");
    exit(0);
  }
  printf("Send pipe: %08x\n", &sPipe[0]);
  p = pipe(rPipe[0]);
  if (p < 0) {
    printf("pipe error");
    exit(0);
  }
  printf("Receive pipe 1: %08x\n", &rPipe[0]);
  p = pipe(rPipe[1]);
  if (p < 0) {
    printf("pipe error");
    exit(0);
  }
  printf("Receive pipe 2: %08x\n", &rPipe[1]);
  int k;
  time_t timer;
  for (k = 0; k < 3; k++){
    if ((pids[k] = fork()) == 0)
      if (k == 0){      /*Start of the store manager code*/
        //init
        int i,j;
        int count = 0;
        printf("s%d Store manager start\n", count++);
        printf("s%d %d\n", count++, getpid());
        printf("s%d Store manager opening files\n", count++);
        init = fopen(argv[1],"r");
        logfile = fopen(argv[2], "a+");
        if (init == NULL) {
          printf("file open failed in store manager\n\n");
          exit(0);
          }
        int size = initSize(init);
        printf("s%d size %d\n", count++, size);
        // TAB = (Table**)malloc(size*sizeof(Table*));
        // printf("s%d Table: %08x\n", count++, TAB);
        for (i = 0; i < size; i++){
          TAB[i].id[0] = '\0';
         // TAB[i].id = malloc(50*sizeof(char));
        }
        for (i = 0; i < size; i++){  
          char temp1[50];// = malloc(50*sizeof(char));
          int temp2;
          // printf("s%d Store manager reading from file\n", count++);
          fscanf(init, "%s %d", temp1, &temp2);
          // printf("s%d Store manager reading from file\n", count++);
          int s = strlen(temp1);
          temp1[s] = '\0';
          //TAB[i].id = temp1;
          strcpy(TAB[i].id, temp1);
          // printf("s%d Store manager id \n", count++);
          TAB[i].value = temp2;
          // printf("s%d Store manager value \n", count++);
          printf("%s %d\n", TAB[i].id, TAB[i].value);
        }
        printf("s%d Store manager closing pipes\n", count++);
        if (close(sPipe[0][1]) == -1)
          printf("close error closing pipe1[1]\n");
        if (close(rPipe[0][0]) == -1)
          printf("close error closing pipe1[1]\n");
        if (close(rPipe[1][0]) == -1)
          printf("close error closing pipe2[0]\n"); 
        printf("s%d Store manager closed pipes\n", count++);
        SendStruct send; 
        SendStruct recieve;
        int dcnt = 0;
        while (dcnt < 2){
          
          // send.id = malloc(50*sizeof(char));
          // recieve.id = malloc(50*sizeof(char));
          // send = (SendStruct*)malloc(sizeof(SendStruct));
          // recieve = (SendStruct*)malloc(sizeof(SendStruct));
          // printf("s%d Pointers send: %d recieve %d\n", count++, send, recieve);
          printf("s%d Store manager read attempt 1\n", count++);
          int origin = 0;
          if ((n = read(sPipe[0][0], &recieve, sizeof(SendStruct))) == 0){
            printf("read error %d", 0);
            // origin = 1;
            // if ((n = read(sPipe[1][0], recieve, sizeof(SendStruct))) == 0){
            //   printf("read error %d", 1);
            // }           
          } 
          printf("s%d Store Manager recieved id %s, length %d\n", count++, recieve.id, strlen(recieve.id));
          // int s = strlen(recieve.id);
          // recieve.id[s] = '\0';
          printf("s%d Store Manager recieved id %s, length %d\n", count++, recieve.id, strlen(recieve.id));
          printf("s%d Store manager read %s of length %d and origin %d\n", count++, recieve.id, n, origin);
          printf("s%d %d %d %c %s %d %d \n", count++, recieve.origin, recieve.p, recieve.op, recieve.id, recieve.value, recieve.result);
          printf("s%d Store manager dcnt %d\n", count++, dcnt);
          bool check = checkDone(recieve.result);
          printf("s%d Store manager checkDone %d\n", count++, check);
          if (check == true){
            printf("%d \n", recieve.result);
            dcnt++;
            //printf("s%d Store manager dcnt %d\n", count++, dcnt);
          }
          else{
            timer = time(NULL);
            printf("s%d Store manager printing to file\n", count++);
            fprintf(logfile, "Store manager recieved at %s from %d %d %c %s %d %d \n", ctime(&timer), recieve.origin, recieve.p, recieve.op, recieve.id, recieve.value, recieve.result);
            printf("s%d Store manager printed to file\n", count++);
            int res;
            printf("s%d Store manager do %c\n", count++, recieve.op);
            if (recieve.op == 'U'){
              if ((res = tableUpdate(recieve.id, recieve.value, size)) == 0)
                printf("s%d Store manager result of operation: %s %d\n", count++, TAB[globalPosition].id, TAB[globalPosition].value);
            }
            else if (recieve.op == 'R'){
              if ((res = tableRead(recieve.id, recieve.value, size)) == 0)
                printf("s%d Store manager result of operation: %s %d\n", count++, TAB[globalPosition].id, TAB[globalPosition].value);
            }
            send.origin = recieve.origin;
            send.p = recieve.p;
            send.op = recieve.op;
            if (res == 0 ){
              strcpy(send.id, TAB[globalPosition].id);
              // send.id = TAB[globalPosition]->id;
              send.value = TAB[globalPosition].value;
            }
            else {
              strcpy(send.id, "Table operation error");
              // send.id = "Table operation error";
              send.value = 9999;
            }
            send.result = res;
            printf("s%d Store manager writes to pipe %d\n", count++, send.origin);
            int back = send.origin - 1;
            printf("s%d Store manager Back =  %d\n", count++, back);
            if (write(rPipe[back][1], &send, sizeof(SendStruct)) != sizeof(SendStruct))
              printf("write error 0");
            printf("s%d Store manager wrote to pipe %d\n", count++, send.origin);
          }
          // printf("s%d Store manager read attempt 2\n", count++);
          // origin = 1;
          // if ((n = read(sPipe[0][0], &recieve, sizeof(SendStruct))) == 0){
          //   printf("read error %d", 0);
          //   origin = 0;
          //   if ((n = read(sPipe[1][0], &recieve, sizeof(SendStruct))) == 0){
          //     printf("read error %d", 1);
          //   }           
          // } 
          // printf("s%d Store manager read %s\n", count++, recieve.id);
          // printf("s%d %d %d %c %s %d %d \n", count++, recieve.origin, recieve.p, recieve.op, recieve.id, recieve.value, recieve.result);
          // printf("s%d Store manager dcnt %d\n", count++, dcnt);
          // if (checkDone(recieve.result)){
          //   printf("%d \n", recieve.result);
          //   dcnt++;
          //   printf("s%d Store manager dcnt %d\n", count++, dcnt);
          // }
          // else{
          //   int res;
          //   if (recieve.op == 'U'){
          //     res = tableUpdate(recieve.id, recieve.value, size);
          //   }
          //   else if (recieve.op == 'R'){
          //     res = tableRead(recieve.id, recieve.value, size);
          //   }
          //   tm_info = localtime(&timer);
          //   strftime(tt, 20, "%F %H:%M:%S", &tm);
          //   fprintf(logfile, "Store manager recieved at %s from %d %d %c %s %d %d \n", tt, recieve.origin, recieve.p, recieve.op, recieve.id, recieve.value, recieve.result);
          //   send.origin = recieve.origin;
          //   send.p = recieve.p;
          //   send.op = recieve.op;
          //   send.id = TAB[globalPosition].id;
          //   send.value = TAB[globalPosition].value;
          //   send.result = recieve.result;
          //   if (write(rPipe[origin][1], &send, sizeof(SendStruct)) != sizeof(SendStruct))
          //     printf("write error 0");
          //   tm_info = localtime(&timer);
          //   strftime(tt, 20, "%F %H:%M:%S", &tm);
          //   fprintf(logfile, "Store manager sent at %s from %d %d %c %s %d %d \n", tt, send.origin, send.p, send.op, send.id, send.value, send.result);
                     
          // }
        }
        fclose(logfile);
        printf("s%d Store manager exiting\n", count++);
        return 0;     
      }
      else{             /*Start of the clients one and two code*/
        int count = 0;
        transfile = fopen(argv[k+2], "r");
        int size = 0;
        size = initSize(transfile);
        printf("P%d %d Size %d\n", k, count++, size);
        int j;
        SendStruct send;
        SendStruct recieve;
        printf("%d\n", getpid());
        printf("P%d %d Client %d start\n", k, count++, k);
        logfile = fopen(argv[2], "a+");
        transfile = fopen(argv[k+2], "r");
        printf("P%d %d Client %d files opened\n", k, count++, k);
        if (close(sPipe[0][0]) == -1)
          printf("close error");
        if (close(rPipe[k-1][1]) == -1)
          printf("close error");
        printf("P%d %d Client %d pipes closed\n", k, count++, k);
        for (j = 0; j < size; j++){
          // send = (SendStruct*)malloc(sizeof(SendStruct));
          // recieve = (SendStruct*)malloc(sizeof(SendStruct));
          // printf("P%d %d Pointers send: %d recieve %d\n", k, count++, send, recieve);
          // send.id = malloc(50*sizeof(char));
          // recieve.id = malloc(50*sizeof(char));
          int val = 0;
          // char* id = malloc(50*sizeof(char));
          char id[50];
          char operation = fgetc(transfile);
          if (operation == '\n'){
            operation = fgetc(transfile);
          }
          if (operation == 'U'){
            fscanf(transfile, "%s %d", id, &val);
          }
          else if (operation == 'R'){
            fscanf(transfile, "%s", id);
          }          
          printf("P%d %d read from file: %c %s %d \n", k, count++, operation, id, val);
          send.op = operation;
          //send.id = id;
          id[strlen(id)] = '\0';
          strcpy(send.id, id);
          printf("P%d %d send.id %s \n", k, count++, id);
          send.value = val;
          printf("P%d %d send.value %d \n", k, count++, send.value);
          send.origin = k;
          printf("P%d %d send.origin %d \n", k, count++, send.origin);
          send.p = getpid();
          printf("P%d %d send.p %d \n", k, count++, send.p);
          send.result = 0;
          printf("P%d %d send.result %d \n", k, count++, send.result);
          timer = time(NULL);
          //send.id = "MICROSOFT";
          //fprintf(logfile, "Client sent at %s from %d %d %c %s %d %d \n", ctime(&timer), send.origin, send.p, send.op, send.id, send.value, send.result);
          printf("P%d %d Client %d write attempt 1; id = %s\n", k, count++, k, send.id);
          printf("P%d %d Client id length %d\n", k, count++, strlen(send.id));
          if (write(sPipe[0][1], &send, sizeof(SendStruct)) != sizeof(SendStruct))
            printf("write error");
          printf("P%d %d Client %d successful write\n", k, count++, k);
          printf("P%d %d Client message length %d\n", k, count++, sizeof(SendStruct));
          printf("P%d %d Client reads\n", k, count++);
          int n = read(rPipe[k-1][0], &recieve, sizeof(SendStruct));
          timer = time(NULL);
          //fprintf(logfile, "Client recieved at %s from %d %d %c %s %d %d \n", ctime(&timer), recieve.origin, recieve.p, recieve.op, recieve.id, recieve.value, recieve.result);
          printf("P%d %d Client recieved %s\n", k, count++, recieve.id);
          fclose(logfile);
          sleep(3);
        }
        // send = (SendStruct*)malloc(sizeof(SendStruct));
        // recieve = (SendStruct*)malloc(sizeof(SendStruct));
        // send.id = malloc(50*sizeof(char));
        // recieve.id = malloc(50*sizeof(char));
        send.origin = k;
        send.p = getpid();
        send.op = 'U';
        strcpy(send.id,"abc");
        send.value = 1;
        send.result = 999;
        if (write(sPipe[k-1][1], &send, sizeof(SendStruct)) != sizeof(SendStruct))
            printf("write error");
        while(1){}
        return 0;
        }
    }
  waitpid(pids[0], NULL, 0);
  printf("Exit of the process %d\n", pids[0]);
  kill(pids[1], SIGKILL);
  kill(pids[2], SIGKILL);
  return 0;
}

int tableUpdate (char* id, int val, int size){
   int i;
   for (i = 0; i < size; i++){
      if (strcmp(id, TAB[i].id) == 0){
         TAB[i].value += val;
         globalPosition = i;
         return 0;
      }
   }
   globalPosition = size;
   return 1;
} 

int tableRead (char* id, int val, int size){
   int i;
   for (i = 0; i < size; i++){
      if (strcmp(id, TAB[i].id) == 0){
         val = TAB[i].value;
         globalPosition = i;
         return 0;
      }
   }
   globalPosition = size;
   return 1;
}

int initSize(FILE* init){
   int size = 0;
   char temp;
   while (!feof(init)){  
      temp = fgetc(init);
      if((temp == '\n') || (temp = '\0'))
         size++;
   }
   fseek(init,0L,SEEK_SET);
   return size;
}


bool checkDone (int buff){
   bool done = false;
   printf("%d \n", buff);
   if (buff == 999){
      done = true;
   }
   printf("%d \n", buff);
   return done;
}