#include "target.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>


target_list global;

//list of strings for dependencies and rules, gets associated with a target
struct stringList_st {
  stringList next;
  char* string;
};
//linked list for targets
struct target_st {
  char* name;
  target_list next;
  stringList depend;
  stringList rules;
};

typedef struct target_st node;

/* create a target, set its parameters, and add to global list */
target* new_target(char* name) {
  //initialize target
  node* n = malloc(sizeof(node));
  if(n != NULL) {
    char* copy = strdup(name);
    n->next = NULL;
    n->name = copy;
    n->depend = NULL;
    n->rules = NULL;

    //append target to list
    target_list* list = &global;
    while(*list != NULL) {
      list = &((*list)->next);
    }
    *list = n;

  }
  return n;
}

/* go through global list and find target by its name*/
target* find_target(char* name) {
  target_list* list = &global;
  while(*list != NULL) {
    if (strcmp(name, (*list)->name) == 0) {
      return (*list);
    }
    list = &((*list)->next);
  }
  return NULL;
}

/* adds dependencies to a target's stringList of dependencies */
void add_depend_target(target* tgt, char* depend) {
  char* copy = strdup(depend);
  stringList n = malloc(sizeof(struct stringList_st));
  if(n != NULL) {
    n->next = NULL;
    n->string = copy;
    stringList* list = &(tgt->depend);
    while(*list != NULL) {

      list = &((*list)->next);
    }
    *list = n;
  }

}

/* adds a rule to a target's stringList of rules */
void add_rule_target(target* tgt, char* rule) {
  stringList n = malloc(sizeof(struct stringList_st));
  if(n != NULL) {
    char* copy = strdup(rule);
    n->next = NULL;
    n->string = copy;

    stringList* list = &(tgt->rules);
    while(*list != NULL) {
      list = &((*list)->next);
    }
    *list = n;
  }
}
/* applies the given function to all strings in a target's stringList of rules */
void for_each_rule(target* tgt, void(*action)(char*)) {
  stringList list = tgt->rules;
  while(list != NULL) {
    action(list->string);
    list = list->next;
  }
}
/* applies the given function to all strings in a target's stringList of dependencies */
void for_each_dependency(target* tgt, void(*action)(char*)) {
  stringList list = tgt->depend;
  while(list != NULL) {
    action(list->string);
    list = list->next;
  }
}

//scans a target's dependency list and returns the time of its most recently
// updated dependency as a time_t. called in recursive_dependencies
time_t findNewestDepend(target* tgt) {
  time_t newestmod;
  stringList list = tgt->depend;
  if (list == NULL)
    return 0;

  //set newest mod time to first dependency
  struct stat fileStat;
  if (stat(list->string, &fileStat) == 0)
    newestmod = fileStat.st_mtime;

  //compare newest mod time to rest of dependencies, updating if necessary
  list = list->next;
  while(list != NULL) {
    struct stat fileStat1;
    if (stat(list->string, &fileStat1) == 0)
      if (fileStat1.st_mtime > newestmod)
        newestmod = fileStat1.st_mtime;
    list = list->next;
  }
  //end of list
  return newestmod;
}
