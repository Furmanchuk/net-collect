#include <stdbool.h>
#include <time.h>

/**
 *Run mode enumeration.
 */
enum run_mode {
    /** Daemon mode running */
    RM_DAEMON,
    /** Statistics mode running */
    RM_STAT };

// DoTo: redeclaration
#pragma once

/**
 * Collected main parameters after parse.
 * @dbfile: database file path.
 * @run_mode: :c:type:`run_mode`
 * @period: update period
 * @rotate: duration of work
 * @netinterface: interface name
 * @limMiB: traffic threshold
 * @commandstr: internal command
 * @from: show data from time
 * @to: show data to time
 */
struct _arguments {
  char *dbfile;
  enum run_mode mode;
  struct dargs {
    long long period;
    long long rotate;
    char *netinterface;
    long long limMiB;
    char *commandstr;
  };
  time_t from;
  time_t to;
};
/** Opaque object implemented with :c:type:`_arguments` */
typedef struct _arguments arguments;

/**
 * args_parce() - parcer input of command line
 * @argc: number of in optins
 * @argv: options of command line
 * @args: :c:type:`arguments`
 * @return: true -- if parse was successful or false -- otherwise
 *
 * Function parced the command line arguments and fill :c:type:`_arguments`
 * according to the entered options and their parameters.
 *
 */
bool args_parce(int argc, char *argv[], arguments *args);
