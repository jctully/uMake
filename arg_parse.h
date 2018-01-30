#ifndef _arg_parse_
#define _arg_parse_

/* Called by processline, takes in a line of commands as a string (char*) and
returns an array of the commands with each position pointing to the first letter of the command. */
 
char **arg_parse(char *line, int *argcp);

#endif
