#include "collectdb.h"

#include <sqlite3.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>


#define DB_TABLE "netcollect"
#define ID_FLD "id"
#define TIME_FLD "time"
#define RX_FLD "rx"
#define TX_FLD "tx"
//%s
static const char *const SQL_QUERY_CREATE_TABLE = (
	"CREATE TABLE " DB_TABLE " ("
	ID_FLD " INT PRIMARY KEY NOT NULL,"
	TIME_FLD " INTEGER,"
	RX_FLD " INTEGER ,"
	RX_FLD " INTEGER );"
);

static const char *const SQL_QUERY_SELECT_FROM = (
	"SELECT * FROM " DB_TABLE " ORDER BY " TIME_FLD ";"
);

static const char *const SQL_QUERY_INSERT_INTO = (
	"INSERT INTO " DB_TABLE " (" TIME_FLD ", " RX_FLD ", " TX_FLD ") "
    "VALUES (%ld, %lld, %lld);"
);

void dbclose(sqlite3 *db)
{
	int rc = sqlite3_close(db);
	if (rc != SQLITE_OK ){
		exit(1);
	}
}

bool dbconnect(sqlite3 *db, char *dbpath, char **err_msg)
{
	int rc = sqlite3_open(dbpath, &db);
	if (rc != SQLITE_OK) {
		*err_msg = strdup(sqlite3_errmsg(db));
        sqlite3_close(db);
        return false;
    }
	return true;
}


bool print_table(char *db_name, int from, int to)
{

}

bool write_to_db(sqlite3 *db, char *db_name, time_t _time, long long rx, long long tx, char **msg)
{
	int rc;
	char *sql = sqlite3_mprintf(SQL_QUERY_INSERT_INTO, _time, rx, tx);
	char *err_msg;

	//sqlite3 *db;
	rc = sqlite3_open(db_name, &db);
	if (rc != SQLITE_OK) {
		*msg = strdup(sqlite3_errmsg(db));
        return false;
    }

	rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
	if (rc != SQLITE_OK ) {
		*msg = strdup(err_msg);
        sqlite3_free(msg);
        return false;
    }

	return true;
}
//conection to db success
