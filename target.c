#include "target.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

target_list global;

/* for debugging
static void print_name(char* string) {
  printf("%s\n", string);
}*/

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
  node* n = malloc(sizeof(node));
  if(n != NULL) {
    char* copy = strdup(name);
    n->next = NULL;
    n->name = copy;

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
  //printf("finding target:  ");
  target_list* list = &global;
  while(*list != NULL) {
    if (strcmp(name, (*list)->name) == 0) {
      //printf("found target\n");
      return (*list);
    }
    list = &((*list)->next);
  }
  return NULL;
}

/* adds dependencies to a target's stringList of dependencies */
void add_depend_target(target* tgt, char* depend) {
  //printf("adding depend:  ");
  stringList n = malloc(sizeof(node));
  if(n != NULL) {
    n->next = NULL;
    n->string = depend;

    stringList* list = &(tgt->depend);
    while(*list != NULL) {
      list = &((*list)->next);
    }
    //printf("added\n");
    *list = n;
  }
}

/* adds a rule to a target's stringList of rules */
void add_rule_target(target* tgt, char* rule) {
  //printf("adding rule:  ");
  stringList n = malloc(sizeof(node));
  if(n != NULL) {
    char* copy = strdup(rule);
    n->next = NULL;
    n->string = copy;

    stringList* list = &(tgt->rules);
    while(*list != NULL) {
      list = &((*list)->next);
    }
    //printf("added\n");
    *list = n;
  }
}
/* applies the given function to all strings in a target's stringList of rules */
void for_each_rule(target* tgt, void(*action)(char*)) {
  //printf("for each rule found: ");
  stringList list = tgt->rules;
  while(list != NULL) {
    action(list->string);
    list = list->next;
  }
}
/* applies the given function to all strings in a target's stringList of dependencies */
void for_each_dependency(target* tgt, void(*action)(char*)) {
  //printf("for each depend found: ");
  stringList list = tgt->depend;
  while(list != NULL) {
    action(list->string);
    list = list->next;
  }
}

/* for testing
void target_test() {
  target_list list = NULL;
  target* tgt = new_target("target 1");
  target* tgt2 = new_target("target 2");
  //find_target("target 1");
  find_target("target 2");
  add_depend_target(tgt, "dep1");
  add_depend_target(tgt, "dep2");
  add_rule_target(tgt, "rule1");
  add_rule_target(tgt, "rule2");
  for_each_rule(tgt, print_name);

} */
