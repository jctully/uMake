/* CSCI 347 micro-make
 *
 * 09 AUG 2017, Aran Clauson
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>

/* CONSTANTS */


/* PROTOTYPES */

/* Process Line
 * line   The command line to execute.
 * This function interprets line as a command line.  It creates a new child
 * process to execute the line and waits for that process to complete.
 */
void processline(char* line);

/* Called by processline, takes in a line of commands as a string (char*) and
returns an array of the commands with each position pointing to the first letter of the command. */

char** arg_parse (char* line);
/* Main entry point.
 * argc    A count of command-line arguments
 * argv    The command-line argument valus
 *
 * Micro-make (umake) reads from the uMakefile in the current working
 * directory.  The file is read one line at a time.  Lines with a leading tab
 * character ('\t') are interpreted as a command and passed to processline minus
 * the leading tab.
 */
int main(int argc, const char* argv[]) {

  FILE* makefile = fopen("./uMakefile", "r");

  size_t  bufsize = 0;
  char*   line    = NULL;
  ssize_t linelen = getline(&line, &bufsize, makefile);

  while(-1 != linelen) {

    if(line[linelen-1]=='\n') {
      linelen -= 1;
      line[linelen] = '\0';
    }

    if(line[0] == '\t')
      processline(&line[1]);


    linelen = getline(&line, &bufsize, makefile);
  }

  free(line);
  return EXIT_SUCCESS;
}


/* Process Line
 *
 */
void processline (char* line) {
  char** args = arg_parse(line);

  const pid_t cpid = fork();
  switch(cpid) {

  case -1: {
    perror("fork");
    break;
  }

  case 0: {
    execvp(args[0], args);
    perror("execvp");
    exit(EXIT_FAILURE);
    break;
  }

  default: {
    int   status;
    const pid_t pid = wait(&status);
    if(-1 == pid) {
      perror("wait");
    }
    else if (pid != cpid) {
      fprintf(stderr, "wait: expected process %d, but waited for process %d",
              cpid, pid);
    }
    break;
  }
  }
  free(args);
}

 /* given a line of commands, the function will return the number of arguments
 as an int, skipping whitespace*/
int countArgs (char* line) {
  int i = 0;
  int onWhitespace = 1;
  int countargs = 0;
  while (line[i] != '\0') {
    if (isspace(line[i])) {
      onWhitespace = 1;
    }
    else if (onWhitespace == 1){
      countargs += 1;
      onWhitespace = 0;
    }
    i++;
  }
  return countargs;
}
/* Arg Parse the command line to parse
 Called by processline, takes in a line of commands as a string (char*) and
returns an array of the commands with each position pointing to the first letter of the command.
To do this it uses a kind of state machine to tell if it is reading whitespace, or moving from
white space to characters. */
char** arg_parse (char* line) {

  int count = countArgs(line);
  char** args = malloc ((count+1) * sizeof(char*));

  int i = 0;
  int onWhitespace = 1;
  int countargs = 0;
  while (line[i] != '\0') {
    if (isspace(line[i])) {
      if (onWhitespace == 0) {
        line[i] = '\0';
      }
      onWhitespace = 1;
    }
    else if (onWhitespace == 1){
      args[countargs] = &line[i];
      countargs += 1;
      onWhitespace = 0;
    }
    i++;
  }
  args[count]=NULL;
  return args;

}
