
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

#include "collectdb.h"
#include "logging.h"
#include "netdaemon.h"

#include <stdio.h>

int main(int argc, char *argv[]) {
  arguments arguments;
  if (!args_parce(argc, argv, &arguments))
    fprintf(stderr, "%s\n", "Error: Pasing failture");

  switch (arguments.mode) {
  case RM_DAEMON:
    log_info("Daemon mode is selected.");
    daemod_run(&arguments);
    break;
  case RM_STAT:
    log_info("Statistics mode is selected.");
    print_db_table(arguments.dbfile, arguments.from, arguments.to);
    break;
  }

  return 0;
}
