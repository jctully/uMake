#ifndef _TARGET_H_
#define _TARGET_H_

#include <stdlib.h>

struct target_st;
typedef struct target_st target;
typedef struct target_st* target_list;

struct stringList_st;
typedef struct stringList_st* stringList;

target* new_target(char* name);
target* find_target(char* name);

void add_depend_target(target* tgt, char* depend);
void add_rule_target(target* tgt, char* rule);

void for_each_rule(target* tgt, void(*action)(char*));
void for_each_dependency(target* tgt, void(*action)(char*));

void target_test();

#endif
