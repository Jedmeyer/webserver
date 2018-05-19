

#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>

#ifndef MAXTOKS
#define MAXTOKS 3


struct name{
char **tok;
int count;
int status;
enum status_value { NORMAL, EOF_OR_ERROR , TOO_MANY_TOKENS};
};

int read_name(struct name *hold){
  size_t nbytes = 100;
  char *my_string = NULL;


  hold->tok = malloc(MAXTOKS * sizeof(char**));


  int bytes_read = getline (&my_string, &nbytes, stdin);
  hold->count=0;


  int count = 0;
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

      hold->tok[hold->count] = malloc((N+1) * sizeof(char));
  
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



int main(){
	printf("Send GET request...\n");
	char *buffer;
	int bufflen = 100;
	FILE *file1;
	FILE *file2;

	struct name input;

	file1 = fopen(input.tok[1], "r");
	if (file1 == NULL)
		perror ("Error opening file");
	while (getline(&buffer, &bufflen, file1) >= 0){
		fprintf(stdout, "%s", buffer);
	    fclose(file1);
	}
	// file2 =  fopen(input[1], "a");
	// if (file2 == NULL) 
 //    	perror ("Error opening file");




}
#endif