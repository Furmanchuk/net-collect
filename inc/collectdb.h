#include <sqlite3.h>
#include <stdbool.h>
#include <time.h>

/**
 *collectdb errors.
 */
enum cerror  {
    CE_OK,      /* No error                                         */
    CE_OE,      /* Cannot open database                             */
    CE_EE,      /* Failed to execute SQL statement                  */
    CE_SE,      /* Failed to select data                            */
    __CE_LAST   /* Last item. So that array sizes match everywhere  */
};

/**
 * print_db_table() - printing collect data in terminal from database in table
 * @db_name: database path
 * @from: from date
 * @to: to date
 * @return: Error :c:type:`cerror`
 */
enum cerror print_db_table(char *db_name, time_t from, time_t to);

/**
 * write_to_db() - writing collect data to database
 * @db: the SQLite database connection object.
 * @db_name: database path
 * @rx: receive bytes
 * @tx: transmit bytes
 * @err_msg: SQLite error message
 * @return: Error :c:type:`cerror`
 */
 enum cerror write_to_db(sqlite3 *db, char *db_name, time_t _time, long long rx,
                 long long tx, char **err_msg);

 /**
  * write_to_db() - create datebese file
  * @db_name: database path
  * @err_msg: SQLite error message
  * @return: Error :c:type:`cerror`
  */
  enum cerror create_db(char *db_name, char **err_msg);
