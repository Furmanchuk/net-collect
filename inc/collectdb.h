#include <sqlite3.h>
#include <stdbool.h>
#include <time.h>

/**
 * print_db_table() - printing collect data in terminal from database in table
 * @db_name: database path
 * @from: from date
 * @to: to date
 * @return: true -- if parse was successful or false -- otherwise
 */
bool print_db_table(char *db_name, time_t from, time_t to);

/**
 * write_to_db() - writing collect data to database
 * @db: the SQLite database connection object.
 * @db_name: database path
 * @rx: receive bytes
 * @tx: transmit bytes
 * @err_msg: SQLite error message
 * @return: true -- if parse was successful or false -- otherwise
 */
bool write_to_db(sqlite3 *db, char *db_name, time_t _time, long long rx,
                 long long tx, char **err_msg);
