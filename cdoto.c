#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// CONSTANTS (PP)
#define TASK_TITLE_LEN 256
#define TASK_DESCRIPTION_LEN 1024
#define TASK_FILE_NAME "cdoto.txt"
#define TASK_PARSE_TEMPLATE "%d:%s:%s:%d\n"
#define TASK_DEFAUL_INIT {0, "Task", "-", 1}
#define TASK_FIELDS_COUNT 4
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
  int status; // 0 - done; 1 - active ; 2 - other;
} Task;

typedef struct tasks_list {
  Task *task;
  struct tasks_list *next;
} TList;

// DECLARATIONS

// System
int file_touch(char *filename);
FILE *file_open_r(char *filename);
FILE *file_open_a(char *filename); // TODO:
FILE *file_open_w(char *filename); // TODO:
int file_read_last_str(FILE *fp, char *str, int str_size);

// Task shit
int task_show(TList *head, char mode);         // TODO:
TList *task_read(char *filename, TList *head); // TODO: Test It Only
int task_gen_id(char *filename);
int task_create(Task *task, int argc, char *argv[]);
int task_file_append(Task *task);
int strtotask(char *str, Task *task);

// DEFINITIONS

int task_show(TList *head, char mode) {
  TList *curr_node = head;
  int tasks_printed = 0;

  if (head == NULL) {
    return -1;
  }

  while (curr_node != NULL) {
    printf("%d - %s - %s - %d\n", curr_node->task->id, curr_node->task->title,
           curr_node->task->description, curr_node->task->status);
    curr_node = curr_node->next;
    tasks_printed++;
  }
  return tasks_printed;
}

FILE *file_open_r(char *filename) {
  FILE *fp = fopen(filename, "r");

  if (fp == NULL) {
    fprintf(stderr, "!=> Can't open file \"%s\" in READ mode.\n", filename);
    return NULL;
  }
  return fp;
}

// Reurns Count of readed tasks
TList *task_read(char *filename, TList *head) {
  FILE *fp = file_open_r(filename);
  if (fp == NULL) {
    return NULL;
  }

  TList *prev_node = NULL;
  TList *curr_node = NULL;
  int tasks_cntr = 0;
  char str_buff[TASK_TITLE_LEN + TASK_DESCRIPTION_LEN + 128];
  char *is_continue;

  while (is_continue) {
    is_continue =
        fgets(str_buff, TASK_TITLE_LEN + TASK_DESCRIPTION_LEN + 128, fp);
    if (is_continue == NULL || str_buff[0] == '\n') {
      break;
    }

    curr_node = (TList *)malloc(sizeof(struct tasks_list));
    curr_node->next = NULL;
    curr_node->task = (Task *)malloc(sizeof(struct task));
    if (head == NULL) {
      head = curr_node;
    }
    strtotask(str_buff, curr_node->task);
    if (prev_node != NULL) {
      prev_node->next = curr_node;
    }
    prev_node = curr_node;

    tasks_cntr++;
  }
  return head;
}

int file_touch(char *filename) {
  FILE *fp;

  if (!(fp = fopen(filename, "a"))) {
    return 1;
  }

  return 0;
}

int task_create(Task *task, int argc, char *argv[]) {
  int c_arg_cntr = 0; // 1 < c_arg_cntr < 3

  task->id = task_gen_id(TASK_FILE_NAME);
  if (task->id == -1) {
    fprintf(stderr, "!=> Error occured while generating ID.\n");
    return 1;
  }
  if (task->id == 0) {
    fprintf(stderr, "!=> Idk how it even possible...\n");
    return 1000 - 7;
  }

  strcpy(task->title, optarg);
  c_arg_cntr++; // 1

  if (optind < argc && argv[optind][0] != '-') {
    strcpy(task->description, argv[optind]);
    c_arg_cntr++; // 2
  }
  return 0;
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

int file_read_last_str(FILE *fp, char *str, int str_size) {
  long long fp_pos;
  int fp_nl_ctr = 0;

  fseek(fp, 0, SEEK_END);
  fp_pos = ftell(fp);

  while (fp_pos > 0 && fseek(fp, --fp_pos, SEEK_SET) == 0) {
    if (fgetc(fp) == '\n') {
      if (fp_nl_ctr >= 1) {
        if (!(fgets(str, str_size, fp))) {
          return 1;
        }
        return 0;
      }
      fp_nl_ctr++;
    }
  }

  if (fp_pos == 0 && fp_nl_ctr == 1) {
    rewind(fp);
    if (!(fgets(str, str_size, fp))) {
      return 1;
    }
  }
  return 0;
}

int strtotask(char *str, Task *task) {
  int things_readed = 0;

  things_readed = sscanf(str, "%d:%255[^:]:%1023[^:]:%d\n", &task->id,
                         task->title, task->description, &task->status);
  if (things_readed != TASK_FIELDS_COUNT) {
    fprintf(stderr,
            "!=> Can't properly parse str -> Task. Expected:%d; Readed:%d.\n",
            TASK_FIELDS_COUNT, things_readed);
    return 1;
  }
  return 0;
}

int task_gen_id(char *filename) {
  FILE *fp;
  char buffer[1024];
  int id;
  Task last_task;

  file_touch(filename);

  if (!(fp = fopen(TASK_FILE_NAME, "r"))) {
    fprintf(stderr, "!=> Can't open task file\n");
    return -1;
  }

  if (file_read_last_str(fp, buffer, 1024)) {
    fprintf(stderr, "!=> Can't read task from file\n");
    return -1;
  }

  if (buffer[0] == '\0') {
    id = 1;
    return id;
  }

  if (strtotask(buffer, &last_task)) {
    return -1;
  }

  id = last_task.id + 1;
  return id;
}

// ENTRY POINT
int main(int argc, char *argv[]) {
  TList *head = NULL; // Init head of TList
  int opt;
  Task new_task = TASK_DEFAUL_INIT;

  if (argc == 1) { // TODO:
    head = task_read(TASK_FILE_NAME, head);
    task_show(head, 0);
    return 0;
  }

  while ((opt = getopt(argc, argv, ARGUMENTS)) != -1) {
    switch (opt) {
    case 'c': {
      if (task_create(&new_task, argc, argv)) {
        fprintf(stderr, "!=> Can't create task.\n");
        return 1;
      }
      if (task_file_append(&new_task)) {
        return 1;
      }
      break;
    }
    default: {
      printf("=> Incorrect option. Try \"%s -h\" for help.\n", argv[0]);
      return 1;
    }
    }
  }
  return 0;
}
