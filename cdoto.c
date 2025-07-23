#include <stdio.h>
#include <unistd.h>

// CONSTANTS (PP)
#define TASK_TITLE_LEN 256
#define TASK_DESCRIPTION_LEN 1024
/*
 * h - help menu
 * c NAME [DESCRIPTION] - create task
 */
#define ARGUMENTS "hc:"

// STRUCTURES
typedef struct task {
  char title[TASK_TITLE_LEN];
  char description[TASK_DESCRIPTION_LEN];
  short status; // 0 - done; 1 - active ; 2 - other;
} Task;

typedef struct tasks_list {
  Task *task;
  struct tasks_list *next;
} TList;

// FUNCTIONS
int task_fill_fields(Task *task) {
  // Iteractions with user
  printf("?> Task Title: ");
  fgets(task->title, TASK_TITLE_LEN, stdin);
  printf("?> Task Description: ");
  fgets(task->description, TASK_DESCRIPTION_LEN, stdin);

  return 0;
}

// ENTRY POINT
int main(int argc, char *argv[]) {
  // Task List stuff
  TList *head = NULL; // Init head of TList
  TList *tlist;       // Chained list of created tasks

  // Task stuff
  static Task new_task = {"Task", "-", 1}; // Status: active
  task_fill_fields(&new_task);
  printf("=> Title: %s=> Description: %s", new_task.title,
         new_task.description);

  return 0;
}
