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
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>

#include "arg_parse.h"
#include "target.h"

/* CONSTANTS */

/* PROTOTYPES */

/* Process Line
 * line   The command line to execute.
 * This function interprets line as a command line.  It creates a new child
 * process to execute the line and waits for that process to complete.
 */
void processline(char* line);

/* Expand
 * orig    The input string that may contain variables to be expanded
 * new     An output buffer that will contain a copy of orig with all
 *         variables expanded
 * newsize The size of the buffer pointed to by new.
 * returns 1 upon success or 0 upon failure.
 *
 * Example: "Hello, ${PLACE}" will expand to "Hello, World" when the environment
 * variable PLACE="World".
 */
int expand(char* orig, char* new, int newsize);


/* recursive_dependencies, given a target name from command line,
comapre its modification time to its dependencies.
If a dependency has its own dependencies, check those recursively. */
void recursive_dependencies(char* name){
  time_t targModTime;
  struct stat fileStat;
  if (stat(name, &fileStat) == 0)
    targModTime = fileStat.st_mtime;

  else {
    target* tgt = find_target(name);
    if (tgt != NULL) {
      for_each_rule(tgt, processline);
      return;
    }
  }

  target* tgt = find_target(name);
  if (tgt != NULL) {
    /*compare target to its newest dependency recursively.
      If one is updated, a chain reaction of updates will start. */
    time_t newestDepend = findNewestDepend(tgt);
    if (targModTime >= newestDepend) {
      for_each_dependency(tgt, recursive_dependencies);
      newestDepend = findNewestDepend(tgt);
      if (targModTime >= newestDepend)
        return;
    }
    for_each_dependency(tgt, recursive_dependencies);
    for_each_rule(tgt, processline);
  }
}

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
  if (makefile == NULL) {
    fprintf(stderr, "no uMakefile, or unable to open.\n");
    exit(EXIT_FAILURE);
  }

  size_t  bufsize = 0;
  char*   line    = NULL;
  ssize_t linelen = getline(&line, &bufsize, makefile);

  target* tempTarg; //will hold last target added

  while(-1 != linelen) {

    if(line[linelen-1]=='\n') {
      linelen -= 1;
      line[linelen] = '\0';
    }

    //remove comments
    if(strchr(line, '#') != NULL)
      *strchr(line, '#') = '\0';

    //case where no indent, want to add target to global list, then add in its dependencies
    if(line[0] != '\t') {
      if(strchr(line, ':') != NULL) {
        int count;
        char* colon = strchr(line, ':');
        colon[0] = ' ';
        char** lineArgs = arg_parse(line, &count);
        tempTarg = new_target(lineArgs[0]);

        for (int i=1; i<count; i++) {
          add_depend_target(tempTarg, lineArgs[i]);
        }

      }
      //case where line contains an equals, treat as environment assignment
      if(strchr(line, '=') != NULL) {
        int count;
        char* equal = strchr(line, '=');
        equal[0] = ' ';
        char** lineArgs = arg_parse(line, &count);
        setenv(lineArgs[0], lineArgs[1], 1);
      }
    }

    //case of tab character, want to add rules to the last target's list
    if(line[0] == '\t') {
      add_rule_target(tempTarg, strdup(line));
    }
    linelen = getline(&line, &bufsize, makefile);

  }

  //execute rules here by calling recursive_dependencies on each command line arg
  for (int i=1; i<argc; i++) {
    recursive_dependencies(argv[i]);
  }

  free(line);
  return EXIT_SUCCESS;
}

/* expand goes through the string orig and finds substrings to expand, denoted
 by ${substring} . These are expanded by environment variables, and saved
 in the string new. expand is called in processline.  */
int expand(char* orig, char* new, int newsize){
  int pos; //holds position of safe string
  int newpointer = 0;//holds position of iterator through new
  char* substring= malloc(10 * sizeof(char*));

  //while unparsed string has more dollar signs, find phrases to expand
  while (strchr(orig, '$')) {

    char* dollarPos = 2+strstr(orig, "${") ;

    pos = (dollarPos - orig) - 2;

    //copying safe string into new
    int j = 0;
    while (j<pos && strlen(new) < newsize)
      new[newpointer++] = orig[j++];

    char* closeBrack = strchr(dollarPos, '}');
    size_t subLen = closeBrack - dollarPos;

    //save string to be expanded as substring
    strncpy(substring, dollarPos, subLen);

    substring[subLen] = '\0';

    //get expansion string
    char* replace = getenv(substring);
    if (replace) {
      //copying replacement phrase into new
      int i = 0;
      while (replace[i] != '\0' && strlen(new) < newsize)
        new[newpointer++] = replace[i++];

      orig = closeBrack+1;
    }
  }
  //copying rest of orig into new
  int i = 0;
  while (orig[i] != '\0' && strlen(new) < newsize)
    new[newpointer++] = orig[i++];

  new[newpointer] = '\0';

  newsize = strlen(new);
  free(substring);
  return 1;
}
/* given arg values, look for symbols that indicate to redirect input or output,
  and perform operation by manipulating file descriptors. */
char** IORedir(char** args) {
  int file;
  for (int i = 0; args[i] != '\0'; i++) {
    //redirect input with fd0
    if (strcmp(args[i], "<") == 0) {
      args[i] = NULL;
      char* filename = args[i+1];
      file = open(filename, O_RDONLY, 0666);
      dup2(file, 0);
      close(file);
    }
    //redirect output, truncate using fd1
    else if(strcmp(args[i], ">") == 0) {
      args[i] = NULL;
      char* filename = args[i+1];
      file = open(filename, O_WRONLY | O_CREAT | O_TRUNC ,0666);
      dup2(file, 1);
      close(file);
    }
    //redirect output, append using fd1
    else if(strcmp(args[i], ">>") == 0) {
      args[i] = NULL;
      char* filename = args[i+1];
      file = open(filename, O_WRONLY | O_CREAT | O_APPEND ,0666);
      dup2(file, 1);
      close(file);
    }
  }
  return args;
}


/* Process Line
 * given a string line of text, process the arguments using arg_parse
 and save the count of arguments. */
void processline (char* line) {
  char* copy = strdup(line); //create copy of line, gets freed below

  //call expand
  char expansion[1024]; // <- change this value to change max size
  int newsize = 1024;
  int expandSuccess = expand(copy, expansion, newsize);
  if (expandSuccess == 0) {
    printf("failed to expand\n");
    return;
  }

  int count;
  char** args = arg_parse(expansion, &count);

    if (count != 0) {// Only run if args are given

      const pid_t cpid = fork();
      switch(cpid) {

      case -1: {
        perror("fork");
        break;
      }

      case 0: {
        args = IORedir(args);
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
  }
  free(copy);
  free(args);
}
