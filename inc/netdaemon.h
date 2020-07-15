#include <stdbool.h>


struct dargs {
    long long period;
    long long rotate;
    char *netinterface;
    long long limMiB;
    char *commandstr;
};

/**
 * daemod_run() - printing collect data in terminal from database in table
 * @args: :c:type:`_arguments`
 * @return: true -- if parse was successful or false -- otherwise
 */
bool daemod_run(char *dbpath, struct dargs *dargs);
