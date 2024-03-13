/*
  utcsh - The UTCS Shell

  <Put your name and CS login ID here>
  Rhea Shah: rheashah
  Ritu Gupta: rigupta
*/

/* Read the additional functions from util.h. They may be beneficial to you
in the future */
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

/* Global variables */
/* The array for holding shell paths. Can be edited by the functions in util.c*/
char shell_paths[MAX_ENTRIES_IN_SHELLPATH][MAX_CHARS_PER_CMDLINE];
static char prompt[] = "utcsh> "; /* Command line prompt */
static char *default_shell_path[2] = {"/bin", NULL};
static char *redirect_symbol = ">";
static int size_of_tokenized_cmdline = MAX_WORDS_PER_CMDLINE; //256
/* End Global Variables */

/* Convenience struct for describing a command. Modify this struct as you see
 * fit--add extra members to help you write your code. */
struct Command {
  char **args;      /* Argument array for the command */
  char *outputFile; /* Redirect target for file (NULL means no redirect) */
  int num_tokens;
  int num_redirects; //only 1 is valid
};

/* Here are the functions we recommend you implement */

char **tokenize_command_line(char *cmdline);
char **tokenize_command_line_for_concurrency(char *cmdline);
struct Command parse_command(char **tokens);
void eval(struct Command *cmd);
int try_exec_builtin(struct Command *cmd);
void exec_external_cmd(struct Command *cmd);
int get_num_toks(char **tokens);

//Ritu driving
void throw_utcsh_error(){
  char emsg[30] = "An error has occurred\n";
  int nbytes_written = write(STDERR_FILENO, emsg, strlen(emsg));
  if(errno == -1) {
    exit(-1);
  }
  if(nbytes_written != strlen(emsg)){
    exit(2); // Shouldn't really happen -- if it does, error is unrecoverable
  }
}

/* Main REPL: read, evaluate, and print. This function should remain relatively
   short: if it grows beyond 60 lines, you're doing too much in main() and
   should try to move some of that work into other functions. */
int main(int argc, char **argv) {
  set_shell_path(default_shell_path);
  if(errno != 0){
    exit(errno); 
  }

  //Rhea driving
  if(argv[1] != NULL){ //enter script mode
    //this means that it's a script, but an invalid one
    if(argv[2] != NULL){ //if more than 2 args
      throw_utcsh_error();
      exit(1);
    }

    else{
      FILE* file = fopen(argv[1], "r");
      if(errno != 0) {
          exit(errno);
      }
      char line[500];
      bool script_empty = true;
      while(fgets(line, sizeof(line), file)){
        //tokenizes input into cmds using helper method (concurrency)
        char **cmds = tokenize_command_line_for_concurrency(line);
        for(int i = 0; i < size_of_tokenized_cmdline; i++){
          if(cmds[i] != NULL){
            if(strlen(cmds[i]) > MAX_WORDS_PER_CMDLINE){ 
              //it's not a good solution, but it works for this project (hack)
              throw_utcsh_error();
              exit(1);
            }

            script_empty = false;

            char **cmd_args = tokenize_command_line(cmds[i]);
            struct Command cmd = parse_command(cmd_args);
            if(cmd.num_tokens > 0){
              eval(&cmd);   
            }
            //free cmd_args
            for(int i = 0; i < size_of_tokenized_cmdline; i++){
              free(cmd_args[i]);
            }
            free(cmd_args);
          }
        }

        //wait loop, for parallel execution 
        for(int i = 0; i < size_of_tokenized_cmdline; i++){
          int status;
          int status_val = wait(&status);
          if(errno == -1) {
           exit(-1);
          }
        }

        //free cmds
        for(int i = 0; i < size_of_tokenized_cmdline; i++){
          free(cmds[i]);
        }
        free(cmds);
      }

      if (script_empty) { //script file is empty (no lines were processed).
        throw_utcsh_error();
        fclose(file);
        if(errno == -1) {
          exit(-1);
        }
        exit(1);
      } else{ //just close; success
        fclose(file);
        if(errno == -1) {
          exit(-1);
        }
        exit(0);
      }
    }
  }

  //Rhea driving
  //command line implementation
  char *input; // Pointer to store the input
  size_t len = 32; // store len of the input
  size_t read; // store # of char read

  input = (char *)malloc(len * sizeof(char)); 
  if (input == NULL) {
     exit(-1);
  }

  while (1) {
    printf("%s", prompt);

    /* Read */
    read = getline(&input, &len, stdin); //to read input
    if(errno == -1) {
        exit(-1);
    }

    input[strlen(input)-1] = '\0'; // removing the last character \n

    /* Evaluate */
    //if fail to read a line of input
    if(read == -1){
      printf("An error has occurred\n");
      exit(1); //exit with one to show error
    }

    //tokenizes input into cmd_args using helper method
    char **cmds = tokenize_command_line_for_concurrency(input);

    for(int i = 0; i < size_of_tokenized_cmdline; i++){
      if(cmds[i] != NULL){
        char **cmd_args = tokenize_command_line(cmds[i]);
        struct Command cmd = parse_command(cmd_args);
        if(cmd.num_tokens > 0){
            eval(&cmd);
        }

        //free cmd_args
        for(int i = 0; i < size_of_tokenized_cmdline; i++){
          free(cmd_args[i]);
        }
        free(cmd_args);
      }
    }

    //wait loop
    for(int i = 0; i < size_of_tokenized_cmdline; i++){
      int status;
      int status_val = wait(&status);
      if(errno == -1) {
        exit(-1);
      }
    }

    //free cmds
    for(int i = 0; i < size_of_tokenized_cmdline; i++){
      free(cmds[i]);
    }
    free(cmds);
  }

  free(input);
  return 0;
}

/* NOTE: In the skeleton code, all function bodies below this line are dummy
implementations made to avoid warnings. You should delete them and replace them
with your own implementation. */

/** Turn a command line into tokens with strtok
 *
 * This function turns a command line into an array of arguments, making it
 * much easier to process. First, you should figure out how many arguments you
 * have, then allocate a char** of sufficient size and fill it using strtok()
 */
char **tokenize_command_line(char *cmdline) {
  //Rhea driving
  //tokenizes input into cmd_args
  char **cmd_args = (char **)calloc(size_of_tokenized_cmdline, sizeof(char*));
  if (cmd_args == NULL) {
     exit(-1);
  }
  int i = 0;
  
  char *tok = strtok(cmdline, " \n\t");
  if(errno == -1) {
    exit(-1);
  }
  
    if(tok != NULL){
      cmd_args[i] = (char *) calloc(1, strlen(tok) + 1);
      if (cmd_args[i] == NULL) {
        exit(-1);
      }
      strcpy(cmd_args[i], tok);
      i++;

      while(tok != NULL){
        tok = strtok(NULL, " \n\t");
        if(errno == -1) {
            exit(-1);
        }
        if(tok != NULL){
          cmd_args[i] = (char *) calloc(1, strlen(tok) + 1);
          if (cmd_args[i] == NULL) {
            exit(-1);
          }
          strcpy(cmd_args[i], tok);
          i++;
        }
      }
    }
    
    return cmd_args;
}

//Ritu driving
/*Tokenizes command line based on & -- multiple commands*/
char **tokenize_command_line_for_concurrency(char *cmdline) {
  //tokenizes input into cmds
  char **cmds = (char **)calloc(size_of_tokenized_cmdline, sizeof(char*));
  if (cmds == NULL) {
    exit(-1);
  }
  int i = 0;
  
  char *tok = strtok(cmdline, "&");
  if(errno == -1) {
    exit(-1);
  }
  if(tok != NULL){
    cmds[i] = (char *) calloc(1, strlen(tok) + 1);
    if (cmds[i] == NULL) {
      exit(-1);
    }
    strcpy(cmds[i], tok);
    i++;

    while(tok != NULL){
      tok = strtok(NULL, "&");
      if(errno == -1) {
        exit(-1);
      }
      if(tok != NULL){
        cmds[i] = (char *) calloc(1, strlen(tok) + 1);
        if (cmds[i] == NULL) {
          exit(-1);
        }
        strcpy(cmds[i], tok);
        i++;
      }
    }
  }
  return cmds;
}

//Ritu driving
/*Returns number of args in a command (toks in tokens)*/
int get_num_toks(char **tokens){
  int i = 0;
  char *tok = tokens[i];
  while(tok != NULL){
      i++;
      tok = tokens[i];
  }
  return i;
}

//Ritu driving
/*Returns number of redirect symbols in a command 
- logically, only 1 is allowed*/
int get_num_redirects(char **tokens){
  int i = 0;
  int num_redirects = 0;
  char *tok = tokens[i];
  while(tok != NULL){
    if(strcmp(tok, redirect_symbol) == 0){
      num_redirects++;
    }
    i++;
    tok = tokens[i];
  }
  return num_redirects;
}

//Ritu driving
/** Turn tokens into a command.
 *
 * The `struct Command` represents a command to execute. This is the preferred
 * format for storing information about a command, though you are free to change
 * it. This function takes a sequence of tokens and turns them into a struct
 * Command.
 */
struct Command parse_command(char **tokens) {
  struct Command cmd = {.args = tokens, .outputFile = NULL, 
  .num_tokens = get_num_toks(tokens), 
  .num_redirects = get_num_redirects(tokens)};
  return cmd;
}

//Rhea driving
/** Determines whether command passed in is built in or an external command.
 * Executes appropriate helper method.
 */
bool is_built_in(struct Command *cmd){
  char *cmd_name = cmd->args[0];
  if((strcmp(cmd_name, "exit") == 0) || (strcmp(cmd_name, "cd") == 0) 
      || (strcmp(cmd_name, "path") == 0)){
    return true;
  }
  return false;
}

//Rhea driving
/** Evaluate a single command
 *
 * Both built-ins and external commands can be passed to this function--it
 * should work out what the correct type is and take the appropriate action.
 */
void eval(struct Command *cmd) {
  if(is_built_in(cmd)){
    try_exec_builtin(cmd);
  } else{
    exec_external_cmd(cmd);
  } 
}

/** Execute built-in commands
*
* If the command is a built-in command, execute it and return 1 if appropriate
* If the command is not a built-in command, do nothing and return 0
*/
int try_exec_builtin(struct Command *cmd) {
  char *cmd_name = cmd->args[0];

  //Ritu driving

  //for the built in command exit
  if(strcmp(cmd_name, "exit") == 0) { //equal strings 
    if(cmd -> args[1] != NULL){ //has args = ERROR
      throw_utcsh_error();
    } else{
      exit(0); //no error, just exit
    }
  } 

  //for the built in command cd
  if(strcmp(cmd_name, "cd") == 0){
    //ensure that there's at least one argument, but not more than 1 argument
    if(cmd -> args[1] != NULL && cmd -> args[2] == NULL){ 
      //only acceptable case (could still have illogical arg)
      int chdir_status = chdir(cmd->args[1]);
      if(errno == -1) {
          exit(-1);
      }

      if(chdir_status == -1){ //errored
        throw_utcsh_error();
      }

      return 1; //success

    } else{ //error - wrong # of args
        throw_utcsh_error();
        return 0; //error
    }
  }

  //Rhea driving
  //for the built in command path
  if (strcmp(cmd_name, "path") == 0){
    char **const paths = cmd->args;
    set_shell_path(paths + 1);
    if(set_shell_path(paths + 1) == 0){
      exit(1); 
    }
    return 1; //success
  }
  return 0;
}


/** Execute an external command
 *
 * Execute an external command by fork-and-exec. Should also take care of
 * output redirection, if any is requested
 */
void exec_external_cmd(struct Command *cmd) {
  //Ritu driving
  __pid_t pid = fork();
  if(errno == -1) {
    exit(-1);
  }
  //int status;

  if(pid == 0){ //is child
    char **argv = (char **)calloc(cmd->num_tokens + 1, sizeof(char*));
    if (argv == NULL) {
      exit(-1);
    }
   
    //Rhea driving
    //Fill the argv array with command and arguments
    for (int i = 0; i < cmd -> num_tokens; i++) {
      argv[i] = (char *) calloc(1, strlen(cmd -> args[i]) + 1);
      if (argv[i] == NULL) {
        exit(-1);
      }
      strcpy(argv[i], cmd -> args[i]);
    }
    argv[cmd -> num_tokens] = NULL; // Null-terminate the cmd->args array

    int num_redirects = cmd->num_redirects;
    if(num_redirects > 1){
      throw_utcsh_error();
      exit(1);
    } else{
      if(num_redirects == 1){ //write to output file
        //if last arg is >
        if(strcmp(cmd->args[cmd->num_tokens - 1], redirect_symbol) == 0){
          throw_utcsh_error();
          exit(1);
        } 
        //if 2nd to last arg is not >
        else if(strcmp(cmd->args[cmd->num_tokens - 2], redirect_symbol) != 0){ 
          throw_utcsh_error();
          exit(1);
        } 
        //first arg is > = invalid
        else if(strcmp(cmd->args[0], redirect_symbol) == 0){
          throw_utcsh_error();
          exit(1);
        }
        else{
          //success case
          cmd->outputFile = cmd->args[cmd->num_tokens- 1];
          //move NULL earlier to only execute up till the > token
          argv[cmd->num_tokens - 2] = NULL; 
        }
      }
    }

    int execv_status = -1; //default error
    char *arg1 = cmd->args[0];
    const char *output_file_name = cmd->outputFile;

    //Ritu driving  
    if(is_absolute_path(arg1)){
      if(output_file_name == NULL){ //abs path w/o redirection
        execv_status = execv(arg1, argv);
        if(errno == -1) {
          exit(-1);
        }
      } else{ //abs path w/ redirection
        // check if directory is created or not
        int fd = open(output_file_name, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if(errno == -1) {
            exit(-1);
        }
        dup2(fd, STDOUT_FILENO); 
        if(errno == -1) {
            exit(-1);
        }
        dup2(fd, STDERR_FILENO);
        if(errno == -1) {
            exit(-1);
        }

        close(fd);
        if(errno == -1) {
          exit(-1);
        }

        execv_status = execv(arg1, argv);
        if(errno == -1) {
            exit(-1);
        }
      }
    
    } else{ //not absolute path, have to append paths
      //Rhea driving
      int i = 0;
      char *currpath = shell_paths[i];
      bool success = false;
      while(i < MAX_ENTRIES_IN_SHELLPATH && currpath != NULL && !success){
        if(exe_exists_in_dir(currpath, arg1, false) != NULL){
          //absolute path after appending
          int len_of_appended_path = strlen(currpath) + strlen(arg1) + 1;
          char *appended_path = 
              (char*) malloc(len_of_appended_path * sizeof(char));
          if (appended_path == NULL) {
            exit(-1);
          }
          joinpath(currpath, arg1, appended_path);

          //Ritu driving
          if(output_file_name != NULL){
            int fd = open(output_file_name, O_WRONLY | O_CREAT | O_TRUNC, 0666);
            if(errno == -1) {
              exit(-1);
            }
            dup2(fd, STDOUT_FILENO); 
            if(errno == -1) {
              exit(-1);
            }
            dup2(fd, STDERR_FILENO);
            if(errno == -1) {
              exit(-1);
            }
            close(fd);
            if(errno == -1) {
              exit(-1);
            }
          }

          execv_status = execv(appended_path, argv);
          if(errno == -1) {
            exit(-1);
          }
          if(execv_status != -1){ //success
            success = true; //once path works, stop iterating through paths
          }

          free(appended_path);
        }
        
        currpath = shell_paths[i];
        i++;
      }
    }
   
    if(execv_status == -1){
      throw_utcsh_error();
      exit(1);
    }

    //Rhea driving
    //free argv
    for(int i = 0; i < cmd->num_tokens + 1; i++){
      free(argv[i]);
    }
    free(argv);
  } //do waiting after eval is called in main
  return;
}