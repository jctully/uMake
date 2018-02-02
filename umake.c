/* CSCI 347 micro-make
 *
 * 09 AUG 2017, Aran Clauson
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <string.h>

#include "arg_parse.h"
#include "target.h"

/* CONSTANTS */

static void print_name(char* string) {
  printf("%s\n", string);
}
/* PROTOTYPES */

/* Process Line
 * line   The command line to execute.
 * This function interprets line as a command line.  It creates a new child
 * process to execute the line and waits for that process to complete.
 */
void processline(char* line);


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

  target* tempTarg;
  while(-1 != linelen) {

    if(line[linelen-1]=='\n') {
      linelen -= 1;
      line[linelen] = '\0';
    }

    //if for not tab, want to add target
    if(line[0] != '\t') {
      if(strchr(line, ':') != NULL) {
        int count;
        char** lineArgs = arg_parse(line, &count);
        char* targName = lineArgs[0];
        targName[strlen(targName)-1] = 0;
        //printf("%s\n", targName);
        tempTarg = new_target(targName);

        for (int i=1; i<count; i++)
          add_depend_target(tempTarg, lineArgs[i]);
      }

    }

    if(line[0] == '\t') {
      //add rules
      add_rule_target(tempTarg, strdup(line));
    }

    linelen = getline(&line, &bufsize, makefile);
  }
  //execute here
  for (int i=1; i<argc; i++) {
    for_each_rule(tempTarg, processline);
    for_each_rule(tempTarg, print_name);
  }
  free(line);
  return EXIT_SUCCESS;
}


/* Process Line
 * given a string line of text, process the arguments using arg_parse
 and save the count of arguments.
 */
void processline (char* line) {
  int count;
  char** args = arg_parse(line, &count);

  if (count == 0)
    return;

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
