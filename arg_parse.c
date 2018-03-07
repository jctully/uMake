#include <stdlib.h>
#include <ctype.h>
#include <string.h>


#include "arg_parse.h"

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
char **arg_parse(char *line, int *argcp) {
  char* copy = strdup(line);

  int count = countArgs(copy);
  *argcp = count;
  char** args = malloc ((count+1) * sizeof(char*));

  int i = 0;
  int onWhitespace = 1;
  int countargs = 0;
  while (copy[i] != '\0') {
    if (isspace(copy[i])) {
      if (onWhitespace == 0) {
        copy[i] = '\0';
      }
      onWhitespace = 1;
    }
    else if (onWhitespace == 1){
      args[countargs] = &copy[i];
      countargs += 1;
      onWhitespace = 0;
    }
    i++;
  }
  args[count]=NULL;
  return args;

}
