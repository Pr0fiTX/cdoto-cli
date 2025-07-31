#include <stdio.h>
#include <string.h>
#include <unistd.h>

// CONSTANTS (PP)
#define TASK_TITLE_LEN 256
#define TASK_DESCRIPTION_LEN 1024
#define TASK_FILE_NAME "cdoto.txt"
#define TASK_PARSE_TEMPLATE "%d:%s:%s:%d\n"
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
int file_touch(char *filename); // TODO:
int file_read_last_str(FILE *fp, char *str, int str_size);

// Task shit
int task_gen_id(char *filename);
Task task_create(int argc, char *argv[]);
int task_file_append(Task *task);
int strtotask(char *str, Task *task);

// DEFINITIONS

int file_touch(char *filename) {
  FILE *fp;

  if (!(fp = fopen(filename, "a"))) {
    return 1;
  }

  return 0;
}

Task task_create(int argc, char *argv[]) {
  Task new_task = {task_gen_id(TASK_FILE_NAME), "Task", "-",
                   1}; // Status: active
  int c_arg_cntr = 0;  // 1 < c_arg_cntr < 3

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

int file_read_last_str(FILE *fp, char *str, int str_size) {
  long long fp_pos = ftell(fp);
  int fp_nl_ctr = 0;

  fseek(fp, 0, SEEK_END);

  while (fp_pos > 0 && fseek(fp, --fp_pos, SEEK_SET) == 0) {
    if (fgetc(fp) == '\n') {
      if (fp_nl_ctr >= 1) {
        if (!(fgets(str, str_size, fp))) {
          return 1;
        }
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

  things_readed = sscanf(str, TASK_PARSE_TEMPLATE, &task->id, task->title,
                         task->description, &task->status);
  if (things_readed != TASK_FIELDS_COUNT) {
    fprintf(stderr,
            "!=> Can't properly parse str -> Task. Expected: %d; Readed: %d.\n",
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
  Task new_task;

  while ((opt = getopt(argc, argv, ARGUMENTS)) != -1) {
    switch (opt) {
    case 'c': {
      new_task = task_create(argc, argv);
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
