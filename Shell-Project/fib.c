#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

const int MAX = 13;

static void doFib(int n, int doPrint);

/*
 * unix_error - unix-style error routine.
 */
inline static void unix_error(char *msg) {
  fprintf(stdout, "%s: %s\n", msg, strerror(errno));
  exit(1);
}

int main(int argc, char **argv) {
  int arg;
  int print = 1;

  if (argc != 2) {
    fprintf(stderr, "Usage: fib <num>\n");
    exit(-1);
  }

  arg = atoi(argv[1]);
  if (arg < 0 || arg > MAX) {
    fprintf(stderr, "number must be between 0 and %d\n", MAX);
    exit(-1);
  }

  doFib(arg, print); //call is made using print = true

  return 0;
}

/*
 * Recursively compute the specified number. If print is
 * true, print it. Otherwise, provide it to my parent process.
 *
 * NOTE: The solution must be recursive and it must fork
 * a new child for each call. Each process should call
 * doFib() exactly once.
 * 
 * 0 = false, 1 = true
 */
static void doFib(int n, int doPrint) {
  //Ritu driving
  pid_t pid1; 
  pid_t pid2; 
  int val1 = 0; 
  int val2 = 0; 
  int status;
  int fibResult;

  if (n == 0 || n == 1){ //base case
    if(doPrint){
      printf("%d\n", n);
      return;
    }
    exit(n); //pseudo return statement
  }

  //Rhea driving
  pid1 = fork();
  if(pid1 == 0){  //if n1 is a child
    doFib(n-1, 0); 
  }

  pid2 = fork();
  if(pid2 == 0){ //if pid2 is a child
    doFib(n-2, 0); 
  }

  waitpid(pid1, &status, 0); //parent waiting for child process 1 to finish
	val1 = WEXITSTATUS(status);
	waitpid(pid2, &status, 0); //parent waiting for child process 2 to finish
	val2 = WEXITSTATUS(status); 
  
  fibResult = val1 + val2; //variable to store where 

  if(doPrint){
    printf("%d\n", fibResult);
  } else{
    exit(fibResult);
  }
}