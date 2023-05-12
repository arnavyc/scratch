#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "subcmd.h"

int a_getopt(sc_subcmd *cmd, sc_subcmd *subcmd, int argc, char *argv[]) {
  bool a = false;
  int b = 0;
  int o;
  while ((o = getopt(argc, argv, "+nht:")) != -1) {
    switch (o) {
      case 'n':
        a = 1;
        break;
      case 't':
        b = atoi(optarg);
        break;
      case 'h':
        fprintf(stderr, "Hello from a!\n");
        subcmd_run_help(stderr, subcmd);
        return 0;
    }
  }

  printf("Hello World! %i %i %i\n", argc, a, b);
  return optind;
}

int b_getopt(sc_subcmd *cmd, sc_subcmd *subcmd, int argc, char *argv[]) {
  bool a = false;
  int b = 0;
  int o;
  while ((o = getopt(argc, argv, "+nht:")) != -1) {
    switch (o) {
      case 'n':
        a = 1;
        break;
      case 't':
        b = atoi(optarg);
        break;
      case 'h':
        fprintf(stderr, "Hello from b!\n");
        subcmd_run_help(stderr, subcmd);
        return 0;
    }
  }

  printf("Hello World! %i %i %i\n", argc, a, b);
  return optind;
}

int main_getopt(sc_subcmd *cmd, sc_subcmd *subcmd, int argc, char *argv[]) {
  bool a = false;
  int b = 0;
  int o;
  while ((o = getopt(argc, argv, "+nht:")) != -1) {
    switch (o) {
      case 'n':
        a = 1;
        break;
      case 't':
        b = atoi(optarg);
        break;
      case 'h':
        fprintf(stderr, "Hello from main!\n");
        subcmd_run_help(stderr, subcmd);
        return 0;
    }
  }

  return optind;
}

int main(int argc, char *argv[]) {
  sc_subcmd cmd_a;
  subcmd_init_leaf(&cmd_a, "a", "Run a", a_getopt);

  sc_subcmd cmd_b;
  sc_subcmd *cmds_b[] = { &cmd_a, NULL };
  subcmd_init_set(&cmd_b, "b", "Run b", b_getopt, cmds_b);

  sc_subcmd cmd_main;
  sc_subcmd *cmds[] = { &cmd_a, &cmd_b, NULL };
  subcmd_init_set(&cmd_main, NULL, NULL, main_getopt, cmds);

  subcmd_run(&cmd_main, argc, argv);
}
