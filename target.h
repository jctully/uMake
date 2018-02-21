#ifndef _TARGET_H_
#define _TARGET_H_
#include <stdlib.h>

//prototypes for target.c
struct target_st;
typedef struct target_st target;
typedef struct target_st* target_list;

struct stringList_st;
typedef struct stringList_st* stringList;

//creates a new target obj and adds to global list. used in find
target* new_target(char* name);

//searches global list for given target name. Used when finding target from command line
target* find_target(char* name);

//add a dependency or rule to a target's stringlist of dependencies or rules
void add_depend_target(target* tgt, char* depend);
void add_rule_target(target* tgt, char* rule);

//apply a given action to a target's rule list or dependency list.
//used to recursively find target's dependencies and execute their rules.
void for_each_rule(target* tgt, void(*action)(char*));
void for_each_dependency(target* tgt, void(*action)(char*));

#endif
