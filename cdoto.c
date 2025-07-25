#include <stdio.h>
#include <string.h>
#include <unistd.h>

// CONSTANTS (PP)
#define TASK_TITLE_LEN 256
#define TASK_DESCRIPTION_LEN 1024
#define TASK_FILE_NAME "cdoto.txt"
#define TASK_PARSE_TEMPLATE "%d:%s:%s:%d\n"
/*
 * h - help menu
 * c NAME [DESCRIPTION] - create task
 */
#define ARGUMENTS "hc:"

// STRUCTURES
typedef struct task {
  int id;
  char title[TASK_TITLE_LEN];
  char description[TASK_DESCRIPTION_LEN];
  short status; // 0 - done; 1 - active ; 2 - other;
} Task;

typedef struct tasks_list {
  Task *task;
  struct tasks_list *next;
} TList;

// FUNCTIONS
Task task_create(int argc, char *argv[]) {
  Task new_task = {0, "Task", "-", 1}; // Status: active
  int c_arg_cntr = 0;                  // 1 < c_arg_cntr < 3

  strcpy(new_task.title, optarg);
  c_arg_cntr++; // 1

  if (optind < argc && argv[optind][0] != '-') {
    strcpy(new_task.description, argv[optind]);
    c_arg_cntr++; // 2
  }
  return new_task;
}

int task_file_append(Task *task) {
  FILE *fp;

  fp = fopen(TASK_FILE_NAME, "a");
  if (!fp) {
    perror("!=> Can't open file to append task.\n");
    return 1;
  }
  fprintf(fp, TASK_PARSE_TEMPLATE, task->id, task->title, task->description,
          task->status);
  fclose(fp);
  return 0;
}

// int task_file_read()

// ENTRY POINT
int main(int argc, char *argv[]) {
  TList *head = NULL; // Init head of TList
  int opt;
  Task new_task;

  while ((opt = getopt(argc, argv, ARGUMENTS)) != -1) {
    switch (opt) {
    case 'c': {
      new_task = task_create(argc, argv);
      if (task_file_append(&new_task)) {
        return 1;
      }

      printf("=> %s : %s\n", new_task.title, new_task.description); // WARN:DBG
      break;
    }
    default: {
      printf("=> Use %s -h for options.\n", argv[0]);
      return 1;
    }
    }
  }
  return 0;
}
