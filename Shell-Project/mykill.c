
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


int main(int argc, char **argv)
{
  //getting the pid from the command line input
  pid_t pid1 = atoi(argv[1]); 
  kill(pid1, SIGUSR1); 
  return 0;
}
