/*
 * Run the daemon that calls the command "ip -s link ls dev <interface name>"
 * returns quantity in bytes of the transferred and received data by the
 * specified interface. This data is then written to the database.
 *
 * (C) 2020 Vadym Furmanchuk
 *
 * Homepage: https://github.com/Furmanchuk/net-collect/
 *
 * License:
 * The MIT License (MIT)
 */


#include <stdbool.h>

/**
 * Daemon data structs.
 * @period: update period
 * @rotate: duration of work
 * @netinterface: interface name
 * @limMiB: traffic threshold
 * @commandstr: internal command
 */
struct dargs {
    long long period;
    long long rotate;
    char *netinterface;
    long long limMiB;
    char *commandstr;
};

/**
 * daemod_run() - run daemon to collect net data
 * @args: :c:type:`dargs`
 */
void daemod_run(char *dbpath, struct dargs *dargs);
