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

/* for debugging
static void print_name(char* string) {
  printf("%s\n", string);
}*/

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
int main(int argc, char* argv[]) {

  FILE* makefile = fopen("./uMakefile", "r");

  size_t  bufsize = 0;
  char*   line    = NULL;
  ssize_t linelen = getline(&line, &bufsize, makefile);

  target* tempTarg; //will hold last target added

  while(-1 != linelen) {

    if(line[linelen-1]=='\n') {
      linelen -= 1;
      line[linelen] = '\0';
    }

    //case where no indent, want to add target to global list, then add in its dependencies
    if(line[0] != '\t') {
      if(strchr(line, ':') != NULL) {//line contains a colon
        int count;
        char* colon = strchr(line, ':');
        colon[0] = ' '; //strip colon
        char** lineArgs = arg_parse(line, &count);//split line into array
        tempTarg = new_target(lineArgs[0]);

        for (int i=1; i<count; i++) //add dependencies for rest of line array
          add_depend_target(tempTarg, lineArgs[i]);
      }
    }

    //case of tab character, want to add rules to the last target's list
    if(line[0] == '\t') {
      add_rule_target(tempTarg, strdup(line));
    }
    linelen = getline(&line, &bufsize, makefile);
  }

  //execute rules here by calling processline on each command line arg
  for (int i=1; i<argc; i++) {
    for_each_rule(find_target(argv[i]), processline);
  }
  free(line);
  return EXIT_SUCCESS;
}


/* Process Line
 * given a string line of text, process the arguments using arg_parse
 and save the count of arguments.
 */
void processline (char* line) {
  char* copy = strdup(line); //create copy of line, gets freed below

  int count;
  char** args = arg_parse(copy, &count);

  if (count == 0)// do nothing if no args
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
  free(copy);
  free(args);
}
