#include <stdbool.h>
#include <time.h>
#include <sqlite3.h>



bool print_db_table(char *db_name, time_t from, time_t to);


bool write_to_db(sqlite3 *db, char *db_name,
    time_t _time, long long rx, long long tx, char **err_msg);
