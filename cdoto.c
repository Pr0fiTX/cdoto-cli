#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// CONSTANTS
#define TASK_TITLE_LEN 256
#define TASK_DESCRIPTION_LEN 1024
#define TASK_FILE_NAME "cdoto.txt"
#define TASK_FIELDS_COUNT 4
// Task Templates
#define TASK_WRITE_TEMPLATE "%d^%s^%s^%d\n"
#define TASK_READ_TEMPLATE "%d^%255[^^]^%1023[^^]^%d\n"
#define TASK_PRINT_TEMPLATE "%d: [%c] %s (%s)\n"
#define TASK_DEFAULT_INIT {0, "Task", "-", ACTIVE}
/*
 * h - help menu
 * c TITLE DESCRIPTION - create task
 * d ID - mark task with this ID as done
 */
#define ARGUMENTS "hc:d:"

const char help_msg[] =
    "HELP MENU:\n\nFlags:\n\t-h -- show this message\n\t-c TITLE DESCRIPTION "
    "-- create new task\n\t-d ID -- mark task as DONE\n";

// STRUCTURES
typedef enum {
  RET_NONVALID = -1, // For Numbers
  RET_SUCCESS = 0,
  RET_ERROR = 1, // For Statuses
  RET_UNDEFBEHAV = 77,
  RET_NOTIMPLEMENTED = 99
} RetCode;

typedef enum { DONE = 0, ACTIVE = 1, EXPIRED = 2, DELETED = 3 } TaskStatus;

typedef struct task {
  int id;
  char title[TASK_TITLE_LEN];
  char description[TASK_DESCRIPTION_LEN];
  TaskStatus status;
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
FILE *file_open_w(char *filename);

// Task&Files
TList *task_read(char *filename, TList *head);
int task_write(char *filename, TList *head); // TODO:
int task_file_append(Task *task);
int file_read_last_str(FILE *fp, char *str, int str_size);

// Tasks Stuff
int task_gen_id(char *filename);
int task_create(Task *task, char *title, char *description);
int strtotask(char *str, Task *task);
int tasktostr(Task *task, char *str); // TODO:

// TList things
Task *task_search_id(int id, TList *head);
int task_show(TList *head, char mode);
void tlist_free(TList *head);

// DEFINITIONS

int task_write(char *filename, TList *head) {
  FILE *fp = file_open_w(filename);
  if (!fp) {
    fprintf(stderr, "!=> Can't open file %s in W mode.\n", filename);
    return RET_ERROR;
  }
  TList *curr = head;

  while (curr != NULL) {
    fprintf(fp, TASK_WRITE_TEMPLATE, curr->task->id, curr->task->title,
            curr->task->description, curr->task->status);
    curr = curr->next;
  }

  return RET_SUCCESS;
}

// Ret: E:NULL S:Pointer To The File
FILE *file_open_w(char *filename) {
  FILE *fp = fopen(filename, "w");

  if (fp == NULL) {
    return NULL;
  }
  return fp;
}

// Ret: E:NULL S:ptr to Task
Task *task_search_id(int id, TList *head) {
  if (id <= 0) {
    return NULL;
  }

  TList *curr = head;

  while (curr != NULL) {
    if (curr->task->id == id) {
      return curr->task;
    }
    curr = curr->next;
  }
  return NULL;
}

void tlist_free(TList *head) {
  TList *next_node;
  TList *node = head;

  while (node != NULL) {
    next_node = node->next;
    free(node->task);
    free(node);
    node = next_node;
  }
}

// Ret: E:RET_NONVALID S:count of printed tasks
int task_show(TList *head, char mode) {
  TList *curr_node = head;
  int tasks_printed = 0;
  char status_char;

  if (head == NULL) {
    return RET_NONVALID;
  }

  while (curr_node != NULL) {
    switch (curr_node->task->status) {
    case ACTIVE:
      status_char = ' ';
      break;
    case DONE:
      status_char = 'x';
      break;
    case DELETED:
      status_char = 'D';
      break;
    case EXPIRED:
      status_char = '~';
      break;
    default:
      return RET_UNDEFBEHAV;
    }

    printf(TASK_PRINT_TEMPLATE, curr_node->task->id, status_char,
           curr_node->task->title, curr_node->task->description);
    curr_node = curr_node->next;
    tasks_printed++;
  }
  return tasks_printed;
}

// Ret: E:NULL S:non-NULL
FILE *file_open_r(char *filename) {
  FILE *fp = fopen(filename, "r");

  if (fp == NULL) {
    fprintf(stderr, "!=> Can't open file \"%s\" in READ mode.\n", filename);
    return NULL;
  }
  return fp;
}

// Ret: E: NULL S: non-NULL
TList *task_read(char *filename, TList *head) {
  FILE *fp = file_open_r(filename);
  if (fp == NULL) {
    return NULL;
  }

  TList *prev_node = NULL;
  TList *curr_node = NULL;
  int tasks_cntr = 0;
  char str_buff[TASK_TITLE_LEN + TASK_DESCRIPTION_LEN + 128];
  char *is_continue = "a"; // Init to prevent undefined behavior

  // Reading Strings & Creating Chained List
  while (is_continue != NULL) {
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

// Ret: E:RET_ERROR S:RET_SUCCESS
int file_touch(char *filename) {
  FILE *fp;

  if (!(fp = fopen(filename, "a"))) {
    return RET_ERROR;
  }

  fclose(fp);
  return RET_SUCCESS;
}

// Ret: E:RET_ERROR S:RET_SUCCESS
int task_create(Task *task, char *title, char *description) {
  int c_arg_cntr = 0; // 1 < c_arg_cntr < 3

  task->id = task_gen_id(TASK_FILE_NAME);
  if (task->id == RET_NONVALID) {
    fprintf(stderr, "!=> Error occured while generating ID.\n");
    return RET_ERROR;
  }

  strcpy(task->title, title ? title : "Task");
  task->title[TASK_TITLE_LEN - 1] = '\0';
  strcpy(task->description, description ? description : "-");
  task->description[TASK_DESCRIPTION_LEN - 1] = '\0';

  return RET_SUCCESS;
}

// Ret: E:RET_ERROR S:RET_SUCCESS
int task_file_append(Task *task) {
  FILE *fp;

  fp = fopen(TASK_FILE_NAME, "a");
  if (!fp) {
    perror("!=> Can't open file to append task.\n");
    return RET_ERROR;
  }
  fprintf(fp, TASK_WRITE_TEMPLATE, task->id, task->title, task->description,
          task->status);
  fclose(fp);
  return RET_SUCCESS;
}

// Ret: E:RET_ERROR S:RET_SUCCESS
int file_read_last_str(FILE *fp, char *str, int str_size) {
  long long fp_pos;
  int fp_nl_ctr = 0;

  fseek(fp, 0, SEEK_END);
  fp_pos = ftell(fp);

  while (fp_pos > 0 && fseek(fp, --fp_pos, SEEK_SET) == 0) {
    if (fgetc(fp) == '\n') {
      if (fp_nl_ctr >= 1) {
        if (!(fgets(str, str_size, fp))) {
          return RET_ERROR;
        }
        return RET_SUCCESS;
      }
      fp_nl_ctr++;
    }
  }

  if (fp_pos == 0 && fp_nl_ctr == 1) {
    rewind(fp);
    if (!(fgets(str, str_size, fp))) {
      return RET_ERROR;
    }
  }
  return RET_SUCCESS;
}

// Ret: E:RET_ERROR S:RET_SUCCESS
int strtotask(char *str, Task *task) {
  int things_readed = 0;

  things_readed = sscanf(str, TASK_READ_TEMPLATE, &task->id, task->title,
                         task->description, &task->status);
  if (things_readed != TASK_FIELDS_COUNT) {
    fprintf(stderr,
            "!=> Can't properly parse str -> Task. Expected:%d; Readed:%d.\n",
            TASK_FIELDS_COUNT, things_readed);
    return RET_ERROR;
  }
  return RET_SUCCESS;
}

// Ret: E:RET_NONVALID S:id for new task [1;...)
int task_gen_id(char *filename) {
  FILE *fp;
  char buffer[1024];
  int id;
  Task last_task;

  file_touch(filename);

  if (!(fp = file_open_r(filename))) {
    return RET_NONVALID;
  }

  // If the file are empty: id = 1
  if (fseek(fp, 0, SEEK_END) == 0 && ftell(fp) == 0) {
    id = 1;
    return id;
  }

  // Else getting last task id
  if (file_read_last_str(fp, buffer, 1024)) {
    fprintf(stderr, "!=> Can't read task from file\n");
    return RET_NONVALID;
  }

  if (strtotask(buffer, &last_task)) {
    return RET_NONVALID;
  }

  fclose(fp);
  id = last_task.id + 1;
  return id;
}

// ENTRY POINT
int main(int argc, char *argv[]) {
  TList *head = NULL; // Init head of TList
  int opt;            // For optarg()

  // -c flag stuff
  // TODO: Alloc them in case of -c
  Task new_task = TASK_DEFAULT_INIT;
  char title[TASK_TITLE_LEN];
  char description[TASK_DESCRIPTION_LEN];

  // -d flag stuff
  Task *found_task;
  int searhing_id;

  if (argc == 1) {
    if (!(head = task_read(TASK_FILE_NAME, head))) {
      fprintf(stderr, "!=> You don't have any tasks yet.\n");
      return RET_ERROR;
    }
    task_show(head, 0);
    tlist_free(head);
    return RET_SUCCESS;
  }

  while ((opt = getopt(argc, argv, ARGUMENTS)) != -1) {
    switch (opt) {
    case 'c':
      // Parsing TITLE
      strncpy(title, optarg, TASK_TITLE_LEN - 1);
      title[TASK_TITLE_LEN - 1] = '\0';
      // Parsing DESCRIPTION
      if (optind < argc) {
        strncpy(description, argv[optind], TASK_DESCRIPTION_LEN);
        description[TASK_DESCRIPTION_LEN - 1] = '\0';
      } else {
        description[0] = '-';
        description[1] = '\0';
      }

      // Creating & Saving new task
      if (task_create(&new_task, title, description)) {
        fprintf(stderr, "!=> Can't create task.\n");
        return RET_ERROR;
      }
      if (task_file_append(&new_task)) {
        return RET_ERROR;
      }
      return RET_SUCCESS;
    case 'h':
      printf("%s", help_msg);
      return RET_SUCCESS;
    case 'd':
      searhing_id = atoi(optarg);
      // Loading tasks from file to the memory
      if (!(head = task_read(TASK_FILE_NAME, head))) {
        fprintf(stderr, "!=> Can't read tasks from file to the memory.\n");
        return RET_ERROR;
      }
      // Searching for needed task
      if (!(found_task = task_search_id(searhing_id, head))) {
        fprintf(stderr, "!=> Invalid ID.\n");
        return RET_ERROR;
      }
      found_task->status = DONE;
      // Writing tasks into the file
      if (task_write(TASK_FILE_NAME, head)) {
        return RET_ERROR;
      }
      // Exit prep.
      tlist_free(head);
      return RET_SUCCESS;
    default:
      printf("=> Incorrect option. Try \"%s -h\" for help.\n", argv[0]);
      return RET_ERROR;
    }
  }
  return RET_SUCCESS;
}
