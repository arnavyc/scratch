#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "subcmd.h"

void subcmd_init_set(sc_subcmd *ctx, const char *name, const char *short_desc, sc_cmd_callback opt_parser, sc_subcmd **subcmds) {
  *ctx = (sc_subcmd){
    .name = name,
    .short_desc = short_desc,
    .cmd_callback = opt_parser,
    .subcmds = subcmds
  };
}

void subcmd_init_leaf(sc_subcmd *ctx, const char *name, const char *short_desc, sc_cmd_callback opt_parser) {
  *ctx = (sc_subcmd){
    .name = name,
    .short_desc = short_desc,
    .cmd_callback = opt_parser
  };
}

static sc_subcmd *sc_get_subcmd(sc_subcmd *ctx, const char *cmd_name) {
  for (sc_subcmd **curr = ctx->subcmds; *curr != NULL; ++curr) {
    if (!strcmp(cmd_name, (*curr)->name)) {
      return *curr;
    }
  }

  return NULL;
}

static void sc_run_subcmd_set(sc_subcmd *cmd, int argc, char *argv[]) {
  if (argc == 0) {
    fprintf(stderr, "No command found on command line.\n");
    exit(EXIT_FAILURE);
  }

  const char *cmd_name = argv[0];
  sc_subcmd *subcmd = sc_get_subcmd(cmd, cmd_name);

  if (subcmd) {
    subcmd_run(subcmd, argc, argv);
  } else {
    fprintf(stderr, "Subcommand not found!\n");
    exit(EXIT_FAILURE);
  }
}

void subcmd_run(sc_subcmd *cmd, int argc, char *argv[]) {
  bool run_subcmd = false;
  if (cmd->cmd_callback != NULL) {
    int opt_count = cmd->cmd_callback(NULL, cmd, argc, argv);
    if (opt_count) {
      argc -= opt_count;
      argv += opt_count;
    } else {
      run_subcmd = true;
    }
  } else {
    --argc;
    ++argv;
  }

  if (cmd->subcmds != NULL && !run_subcmd) {
    sc_run_subcmd_set(cmd, argc, argv);
  }
}

void subcmd_run_help(FILE *fp, sc_subcmd *cmd) {
  sc_subcmd **subcmds = cmd->subcmds;
  if (subcmds) {
    fprintf(stderr, "Sub-commands:\n");
    for (sc_subcmd **cur = subcmds; *cur != NULL; ++cur) {
      fprintf(fp, "  %s\n", (*cur)->name);
      if ((*cur)->short_desc) {
        fprintf(fp, "      %s\n", (*cur)->short_desc);
      }
    }
  }
}
