#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include "util.h"

/*
 * Signal handler that prints "Nice try" and re-enters its loop
 * when given the SIGINT signal.
 *
 * parameter sig = signal associated w/ signal handler.
 */

//Rhea driving
void handler1(int sig) {
  ssize_t bytes;
  const int STDOUT = 1;
  bytes = write(STDOUT, "Nice try.\n", 10);
  if(bytes != 10){
    exit(-999);
  }
}

//Ritu driving  
void handler2(int sig){
  ssize_t bytes;
  const int STDOUT = 1;
  bytes = write(STDOUT, "Exiting.\n", 8);
  printf("%s\n", " ");
  if(bytes != 10){
    exit(-999);
  }
  exit(1); 
}

/*
 * First, print out the process ID of this process.
 *
 * Then, set up the signal handler so that ^C causes
 * the program to print "Nice try.\n" and continue looping.
 *
 * Finally, loop forever, printing "Still here\n" once every
 * second.
 */
int main(int argc, char **argv){
  //Rhea driving
  __pid_t pid = getpid();
  printf("%d\n", pid);

  struct timespec req = {1, 0};
	struct timespec rem;

  signal(SIGINT, handler1);
  signal(SIGUSR1, handler2);

  //Ritu driving
  while(1){
    printf("%s\n", "Still here.");
    int sleep = nanosleep(&req, &rem);
    if(sleep == -1){
      nanosleep(&req, &rem);
    }
  }


  return 0;
}


