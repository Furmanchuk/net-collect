#include "netdaemon.h"
// #include "argparser.h"
#include "logging.h"

#include <stdio.h>

int main(int argc, char *argv[])
{

    struct arguments arguments;

    if (!args_parce(argc, argv, &arguments))
        fprintf(stderr, "%s\n", "Error: Pasing failture");

    switch(arguments.mode){
    case RM_DAEMON:
        log_info("Daemon mode is selected.");
        daemod_run(&arguments);
        break;
    case RM_STAT:
        log_info("Statistics mode is selected.");
        break;
    }


    return 0;
}
