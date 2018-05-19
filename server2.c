/* Server-side use of Berkeley socket calls -- receive one message and print 
   Requires one command line arg:  
     1.  port number to use (on this machine). 
   RAB 3/12 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>

#include "queue.c"



#define NWORKERS 1
#define MAXBUFF 100



#ifndef MAXTOKS
#define MAXTOKS 10


pthread_mutex_t f_lock;
pthread_mutex_t log_lock;
pthread_mutex_t head_lock;

char *prog;
int kill = 0;
struct work_queue wq;




struct name{
char **tok;
int count;
int status;
enum status_value { NORMAL, EOF_OR_ERROR , TOO_MANY_TOKENS};
};

void logwrite(int r_id, int sock_id){
  
    pthread_mutex_lock(&log_lock);
    int ret; 

    //Timstamp and logfile setup
    time_t now = (NULL);
    char *timestamp = malloc(30 * sizeof(char));
    if (strftime(timestamp, 30, "%a, %d %b %Y %T %Z", gmtime(&now)) == 0){
      printf("Unable to retrieve time!!");
    }
    FILE *log = fopen("server_log.txt", "a");
    fprintf(log, "%s : ", timestamp);
    char *req_log;
    req_log =  "Request Recieved from: ";


    //Socket stuff for recording IP addresses    
    struct sockaddr *addr = malloc(sizeof(struct sockaddr));
    socklen_t addrlen = sizeof(struct sockaddr);
    getsockname(sock_id, addr, &addrlen );
    struct sockaddr_in *sin = (struct sockaddr_in *)addr;
    char *ip = inet_ntoa(sin->sin_addr);
    fprintf(log, "%s %s\n", req_log, ip);


    //Cleanup!
    free(timestamp);
    free(addr);
    fclose(log);
    pthread_mutex_unlock(&log_lock);
    return NULL;
}


int read_name(struct name *hold, char *buff){
  size_t nbytes = 100;
  char *my_string = NULL;
  int len;
  for(len =0; buff[len] != '\0'; ++len );

  my_string = malloc(sizeof(char) * len + 1);
  hold->tok = malloc(MAXTOKS * sizeof(char**));
  my_string = buff;

  //Checks for lenth of array. Used to calculate bytes_read;
  int bytes_read = len * sizeof(char); 
  hold->count=0;


  int count = 0;
  printf("bytes read: %d\n", bytes_read);
  while(count<bytes_read){
    if(hold->count >= MAXTOKS && count < bytes_read) {
      break;
    }  
    if(isspace(my_string[count]) || my_string[count] == '\0') {
      count++;
      continue;
    }else {
      int N = 0;
      while(!isspace(my_string[N + count]))
        N++;

        hold->tok[hold->count] = malloc((N) * sizeof(char));
  
        int i = 0;
        while(i < N) {
          hold->tok[hold->count][i] = my_string[i + count];
          i++;
        }

        hold->tok[hold->count][N] = '\0';
        count += N;
        hold->count++;
    }
  }
  if (bytes_read == -1)
    {
      hold->status=EOF_OR_ERROR;
    }
  else
    { 
      hold->status=NORMAL;
    }


  if(hold->status == NORMAL)
    return 1;
  else if (hold->status == TOO_MANY_TOKENS)
    return 2;
  else
    return 0;
}

void send_header(int sock_fd){
  FILE *f1;

  f1 = fopen("header.txt", "r");
  int head_bufflen = 100 + 1;
  int ret  = 0;
  //Send Header from File its kept in.
  while(1){
    char *head_buff = malloc(100 * sizeof(char) );
    int numbytes = getline(&head_buff, &head_bufflen, f1);
    head_buff[numbytes] = '\n';
    if((ret = send(sock_fd, head_buff, numbytes,NULL)) < 0){
        printf("Unable to send!\n");
        perror("send()");
    }
    free(head_buff);
    }
  fclose(f1);

  }


int validate(struct name *req){
  if(strcmp(req->tok[0],"GET") == 0){
    if((strcmp(req->tok[2],"HTTP/1.1")) != 0){
      return 6;
    }
    if((strcmp(req->tok[3],"Host:" == 0)))
      return 1;  
  }
  if((strcmp(req->tok[0],"QUIT")) == 0){
    if(strcmp(req->tok[1], "password") == 0)
      return -1;
  }
     
  return 0;
}

void * process_requests(void* ID){

    int id = ((int)ID);
    
  while(1){

    struct tdata *td2;
    //Bug is here. seems to be waiting at removeq. Not recieving sent message...?
    struct qnode *temp = removeq(&wq);
    td2 = malloc(sizeof(struct tdata));
    printf("Qnode: Socket_FD: %d\n", temp->key);
    td2->csockd = temp->key;
    td2->request_id = temp->id;

    printf("tdata: Socket_FD: %d\n",td2->csockd);
    char buff[MAXBUFF];  /* message buffer */
    int ret;  /* return value from a call */
    printf("New thread started\n");
    
    //Log write disabled due to file weird-ness.
    logwrite(id, td2->csockd);
    if ((ret = recv(td2->csockd, buff, MAXBUFF-1, 0)) < 0) {
      printf("%s ", prog);
      perror("recv()");
      return 1;
    }

    buff[ret] = '\0';  // add terminating nullbyte to received array of char
    size_t num_char = ret;

    printf("Received message (%d chars):\n%s\n", ret, buff);
    //Here parse the returned message.
    struct name input;

    read_name(&input, &buff);
    if(kill == -1){
      if ((ret = close(td2->csockd)) < 0) {
        printf("%s ", prog);
        perror("close(td2->csockd)");
        return 1;
      }
     break; 
    }
    if(validate(&input) == -1){
      if ((ret = close(td2->csockd)) < 0) {
        printf("%s ", prog);
        perror("close(td2->csockd)");
        return 1;
      }
      kill = -1;
      break;  
    }
    if(validate(&input) == 6)
      printf("Request type not supported!\n");
    if(validate(&input))
      printf("GET request valid!\n");


   



    /* Now send the client the HTML file! */
    FILE *file1;
    printf("GET %s", input.tok[1]);

    
    //File Read CRITical section.
    pthread_mutex_lock(&f_lock);
    file1 = fopen(input.tok[1], "r");
    if (file1 == NULL){
      perror ("Error opening file");
      //Here implement the classic #404 thing.
      file1 = fopen("notfound.html", "r");
    }
    pthread_mutex_unlock(&f_lock);


    //Send Header!
    send_header(td2->csockd);

    //Send requested file;
    while(1){
      char *buff2;
      int bufflen = 100;
      buff2 = malloc(bufflen + 1 * sizeof(char));

      //Read in file
      int num_read = getline(&buff2, &bufflen, file1);
      printf("%s", buff2);
      if (num_read <= 0){
        printf("File Sent\n");
        break;
      }
      //Send file (bufflen + 1) bytes at a time
      buff2[num_read] = '\n';
      if((ret = send(td2->csockd, buff2, num_read,NULL)) < 0){
        printf("Unable to send!\n");
        perror("send()");
        }
      free(buff2);
    }

    if ((ret = close(td2->csockd)) < 0) {
      printf("%s ", prog);
      perror("close(td2->csockd)");
      return 1;
    }

  fclose(file1);
  }
  return 0;

}  




int main(int argc, char **argv){
  //Initiazlie Mutexes
  pthread_mutex_init(&q_lock, NULL);
  pthread_cond_init(&qEmpty, NULL); 
  pthread_mutex_init(&log_lock,NULL);
  pthread_mutex_init(&head_lock, NULL);

  
  int ret; 

  if (argc < 2) {
      printf("Usage:  %s port\n", prog);
      return 1;
  }



  //Initialize p-thread variables;
  pthread_t tHandles[NWORKERS];
  int id[NWORKERS];
  for(int i = 0; i <NWORKERS; ++i)
    id[i] = i;

  
  for(int t = 0; t < NWORKERS; ++t){ 
    pthread_create(&tHandles[t], //Thread Handle...?
                    NULL, //Attribute (NOT USED)
                    process_requests, //Function Thread should run
                    (void*)&id[t]); // Arg for process_requests
  }

  struct tdata *tdp;
  int port; 
  int socket_list[1000];
  int sd_num = 0;
  int serverd;  /* socket descriptor for receiving new connections */
  prog = argv[0];
 
  //Port override!
  port = atoi(argv[1]);
  

  struct sockaddr_in sa;
  sa.sin_family = AF_INET;
  sa.sin_port = htons(port);
  sa.sin_addr.s_addr = INADDR_ANY;
  struct sockaddr_in ca;
  int size = sizeof(struct sockaddr);


  if ((serverd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      printf("%s ", prog);
      perror("socket()");
      return 1;
    }

  if (bind(serverd, (struct sockaddr*) &sa, sizeof(sa)) < 0) {
      printf("%s ", prog);
      perror("bind()");
      return 1;
  }


  int pid;
  long int request_id = 0;
  while(1){

    int client_sd;  /* socket descriptor for communicating with client */
    
    struct tdata td;
      

   
    if (listen(serverd, 5) < 0) {
      printf("%s ", prog);
      perror("listen()");
      return 1;
    }

    

    printf("Waiting for a incoming connection...\n");
    if ((client_sd = accept(serverd, (struct sockaddr*) &ca, &size)) < 0) {
      printf("%s ", prog);
      perror("accept()");
      return 1;
    }

    //Okay, so the accepted connection ISNT being passed properly to the td.csockd, as seen in the process requests function.
    td.csockd = client_sd; //Puts socket descriptor in data struct to be passed to thread
    td.request_id = request_id; //Put in client request ID;

    printf("New client on SD-%d\n", client_sd);
    addq(client_sd, request_id, &wq);

    request_id++;
    if(kill == -1)
      break;

    }

    
  printf("Kill command sent. Shutting down...\n");

  if ((ret = close(serverd)) < 0) {
    printf("%s ", prog);
    perror("close(serverd)");
  } 
  
  for(int i = 0; i < NWORKERS ; ++i)
  pthread_join(tHandles[i], NULL);
  
  }


#endif
