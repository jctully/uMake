# uMake
A makefile processor, complete with support for environment variables, recursive target processing, checking dependency update time, and in-line comments.

compile: 
gcc umake.c arg_parse.c target.c -o umake

run: Will run commands given in any file named uMakefile. Example uMakefile given, can be run with:

./umake umake

to make itself. If umake.c is edited, running this command will detect an updated dependency and rebuild umake.o from umake.c before building umake executable.
