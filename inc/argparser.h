#include <stdbool.h>
#include <time.h>

/**
 * enumeration run_mode - Run mode enumeration.
 */
enum run_mode { RM_DAEMON, RM_STAT };

// DoTo: redeclaration
#pragma once

/**
 * struct _arguments - contain main parameters after parse.
 * @dbfile: Description of member1.
 * @run_mode:
 * @period: update period
 * @rotate: duration of work
 * @netinterface: interface name
 * @limMiB: traffic threshold
 * @commandstr: internal command
 * @from: show data from time
 * @ещ: show data to time
 */

struct _arguments{
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

/**
 * typedef arguments - stract argument of argp.
 */
typedef struct _arguments arguments;

/**
 * args_parce() - parcer input of command line.
 * @argc: number of in optins.
 * @arg2: options of command line.
 *
 the function selects command line arguments and populates structure 2 according
  to the entered parameters
 * Function parced the command line arguments and fill structure arguments
 * according to the entered options and their parameters.
 *
 * Return:
 * -true if parse was successful
 * -false otherwise
 */
bool args_parce(int argc, char *argv[], arguments *args);
