#ifndef SC_SUBCMD_H
#define SC_SUBCMD_H

#include <stdio.h>

typedef struct sc_subcmd sc_subcmd;

typedef int (*sc_cmd_callback)(sc_subcmd *cmd, sc_subcmd *subcmd, int argc, char *argv[]);

struct sc_subcmd {
  const char *name;
  const char *short_desc;

  sc_cmd_callback cmd_callback;

  sc_subcmd **subcmds;
};

void subcmd_init_set(sc_subcmd *ctx, const char *name, const char *short_desc, sc_cmd_callback opt_parser, sc_subcmd **subcmds);

void subcmd_init_leaf(sc_subcmd *ctx, const char *name, const char *short_desc, sc_cmd_callback opt_parser);

void subcmd_run(sc_subcmd *ctx, int argc, char *argv[]);

void subcmd_run_help(FILE *fp, sc_subcmd *cmd);

#endif /* SC_SUBCMD_H */
