/*
 * CLI program for collecting statistic of internet traffic. It is possible
 * to collect statistics for a certain period of time and displaying it
 * in to the terminal. It is also possible to call a internal command to
 * notify you that the limit has been exceeded.
 *
 * Can be built with makefile: meke
 *
 * (C) 2020 Vadym Furmanchuk
 *
 * Homepage: https://github.com/Furmanchuk/net-collect/
 *
 * License:
 * The MIT License (MIT)
 */
#define _XOPEN_SOURCE 700

#include "collectdb.h"
#include "logging.h"
#include "netdaemon.h"
//#include "argparser.h"

#include <stdio.h>

#include <argp.h>
#include <libgen.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static const char *DATE_FORMAT = "%Y %m %d %H:%M:%S";

static const char *USAGE =
    "Try `%s--help' or `%1$s --usage' for more information.\n";

/**
 *Run mode enumeration.
 */
enum run_mode {
    /** Daemon mode running */
    RM_DAEMON,
    /** Statistics mode running */
    RM_STAT };

/**
 * Collected main parameters after parse.
 * @dbfile: database file path.
 * @run_mode: :c:type:`run_mode`
 * @dargs: :c:type:`dargs`
 * @from: show data from time
 * @to: show data to time
 */
struct _arguments {
    char *dbfile;
    enum run_mode mode;
    struct dargs dargs;
    time_t from;
    time_t to;
};
/** Opaque object implemented with :c:type:`_arguments` */
typedef struct _arguments arguments;

enum error {
  AE_OK = 0,   /* No error                                         */
  AE_NOARGS,   /* No arguments provided                            */
  AE_WRONGARG, /* Argument has wrong format                        */
  AE_NENARGS,  /* Not enough arguments                             */
  AE_TMARGS,   /* Too much arguments                               */
  AE_CVGERR,   /* Convergence error                                */
  AE_MPERR,    /* Mode parser error                                */
  AE_NODBS,    /* No database specified                            */
  AE_NPIFS,    /* No interface specified                           */
  __AE_LAST    /* Last item. So that array sizes match everywhere  */
};

static const char *const error_msg[] = {[AE_OK] = "",
                                        [AE_NOARGS] = "No arguments provided",
                                        [AE_WRONGARG] = "Wrong argument '%s'",
                                        [AE_NENARGS] = "Too few arguments",
                                        [AE_TMARGS] = "Too many arguments",
                                        [AE_CVGERR] = "Convergence unreachable",
                                        [AE_MPERR] = "Wrong mode",
                                        [AE_NODBS] = "No database specified",
                                        [AE_NPIFS] = "No interface specified",
                                        [__AE_LAST] = NULL};

/* Program documentation. */
static char doc[] = "CLI programm to collect net statistsic\
\vUsega example: \n\
for statistsic mode: ./net-collect stat --db=./some.db\n\
for daemon mode: ./net-collect daemon --db=./some.db --interface=wlp3s0\n\
\n\
Report bugs to <furmanchuk.v.y@gmail.com>";

static struct argp_option options[] = {
    {"db", 'd', "PATH", 0, "Datebase path"},
    {0, 0, 0, 0,
     "The following options should be grouped together in daemon mode:"},
    {"interface", 'i', "NAME", 0, "Network interface name"},
    {"period", 'p', "SECOND", OPTION_ARG_OPTIONAL,
     "The period of writing data "},
    {"rotate", 'r', "MINUTE", OPTION_ARG_OPTIONAL, "Running time"},
    {"limit", 'l', "MiB", OPTION_ARG_OPTIONAL, "Data limit"},
    {"cmd", 'c', "STRING", OPTION_ARG_OPTIONAL, "Internal command"},
    {0, 0, 0, 0,
     "The following options should be grouped together in statistics mode:"},
    {"from", 'f', "DATE", OPTION_ARG_OPTIONAL,
     "From which date. Format[YYYY MM DD HH:MM:SS]"},
    {"to", 't', "DATE", OPTION_ARG_OPTIONAL,
     "Until which date. Format[YYYY MM DD HH:MM:SS]"},
    {0}};

// mode of run anction

static const char *const run_mode_str[] = {
    [RM_DAEMON] = "daemon", [RM_STAT] = "stat"};

static inline bool ld_conv(const char *str, long long *dstptr) {
  char etc;
  log_dbg("in conv");
  return (sscanf(str, "%lld%c", dstptr, &etc) == 1) && (0 < *dstptr);
}

static bool time_conv(const char *str, time_t *t) {
  struct tm tm;
  char *s = strptime(str, DATE_FORMAT, &tm);
  if (NULL == s)
    return false;
  return (-1 != (*t = mktime(&tm)));
}

static bool mode_parser(const char *str, enum run_mode *mode) {
  if (!strcmp(str, run_mode_str[RM_DAEMON])) {
    *mode = RM_DAEMON;
    return true;
  } else if (!strcmp(str, run_mode_str[RM_STAT])) {
    *mode = RM_STAT;
    return true;
  }
  return false;
}

void err_report(enum error err, struct argp_state *state) {
  fprintf(stderr, "Error: %s.\n", error_msg[err]);
  argp_usage(state);
}

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  arguments *args = state->input;
  switch (key) {
  case 'd':
    args->dbfile = arg;
    log_dbg("DataBase file %s", arg);
    break;
  case 'i':
    args->dargs.netinterface = arg;
    log_dbg("Network interface name %s", arg);
    break;
  case 'p':;
    long long period;
    if (!ld_conv(arg, &period) || ((long long)period < 0))
      err_report(AE_CVGERR, state); // return
    args->dargs.period = period;
    log_dbg("Update interval %d[second]", period);
    break;
  case 'r':;
    long long rotate;
    if (!ld_conv(arg, &rotate) || ((long long)rotate < 0)) {
      err_report(AE_CVGERR, state);
    }
    args->dargs.rotate = rotate;
    log_dbg("Update rotete period %d [minute]", rotate);
    break;
  case 'l':;
    long long limMiB;
    if (!ld_conv(arg, &limMiB) || ((long long)limMiB < 0))
      err_report(AE_CVGERR, state); // return
    args->dargs.limMiB = limMiB;
    log_dbg("Limit %lld [MiB]", limMiB);
    break;
  case 'c':
    args->dargs.commandstr = arg;
    log_dbg("Internal command %s", arg);
    break;
  case 'f':;
    time_t from;
    if (!time_conv(arg, &from))
      err_report(AE_CVGERR, state);
    args->from = from;
    log_dbg("from date %ld", from);
    break;
  case 't':;
    time_t to;
    if (!time_conv(arg, &to))
      err_report(AE_CVGERR, state);
    log_dbg("to date %ld", to);
    args->to = to;
    break;
  case ARGP_KEY_NO_ARGS:
    err_report(AE_NOARGS, state);
  case ARGP_KEY_ARG:
    if (state->arg_num > 0)
      err_report(AE_TMARGS, state);

    enum run_mode mode;
    if (!mode_parser(arg, &mode))
      err_report(AE_MPERR, state);
    args->mode = mode;
    log_dbg("Mode arg %s", arg);
    break;
  default:
    return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

static char args_doc[] = "MODE [daemon] \nMODE [stat]";

static struct argp argp = {options, parse_opt, args_doc, doc};

bool args_parce(int argc, char *argv[], arguments *args) {
  /*Set default values*/
  args->dbfile = NULL;
  args->dargs.period = 5;
  args->dargs.rotate = 1;
  args->dargs.netinterface = NULL;
  args->from = 0;
  args->to = 0;
  bool ret = argp_parse(&argp, argc, argv, 0, 0, args) == 0;

  char *progname = basename(argv[0]);
  if (NULL == args->dbfile) {
    fprintf(stderr, "Error: %s. \n", error_msg[AE_NODBS]);
    fprintf(stderr, USAGE, progname);
    exit(AE_NODBS);
  }
  if ((NULL == args->dargs.netinterface) && (RM_STAT != args->mode)) {
    log_err("Error: %s", error_msg[AE_NPIFS]);
    fprintf(stderr, "Error: %s.\n", error_msg[AE_NPIFS]);
    fprintf(stderr, USAGE, progname);
    exit(AE_NPIFS);
  }
  log_dbg("==================");
  log_dbg("Out db: %s", args->dbfile);
  log_dbg("Out mode: %d", args->mode);
  log_dbg("Out period: %lld", args->dargs.period);
  log_dbg("Out rotate: %lld", args->dargs.rotate);
  log_dbg("Out ninterf: %s", args->dargs.netinterface);
  log_dbg("Out limDB: %lld", args->dargs.limMiB);
  log_dbg("Out cmd: %s", args->dargs.commandstr);
  log_dbg("Out from: %ld", args->from);
  log_dbg("Out to: %ld", args->to);

  return ret;
}

int main(int argc, char *argv[])
{
    arguments arguments;
    if (!args_parce(argc, argv, &arguments))
        fprintf(stderr, "%s\n", "Error: Pasing failture");

    switch (arguments.mode) {
    case RM_DAEMON:
        log_info("Daemon mode is selected.");
        daemod_run(arguments.dbfile, &arguments.dargs);
        break;
    case RM_STAT:
        log_info("Statistics mode is selected.");
        print_db_table(arguments.dbfile, arguments.from, arguments.to);
        break;
    }
    return 0;
}
