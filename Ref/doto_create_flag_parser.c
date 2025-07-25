#include <stdio.h>
#include <unistd.h>

/*
 * h - help menu
 * c NAME [DESCRIPTION]
 */
#define ARGUMENTS "hc:"

struct c_arg {
  char *title;
  char *description;
};

int main(int argc, char *argv[]) {
  int opt;

  while ((opt = getopt(argc, argv, ARGUMENTS)) != -1) {
    switch (opt) {
    case 'c': {
      struct c_arg demo;
      int c_arg_cntr = 0; // 1 < c_arg_cntr < 3

      demo.title = optarg;
      c_arg_cntr++; // 1

      if (optind < argc) {
        demo.description = argv[optind];
        c_arg_cntr++; // 2
      }
      printf("=> %s : %s\n", demo.title, demo.description);
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
