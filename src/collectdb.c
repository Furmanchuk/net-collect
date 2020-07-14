#include "collectdb.h"
#include "logging.h"

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

#define N_BUFF 20

static const char *DATE_FORMAT = "%Y %m %d %H:%M:%S";
//%s

static const char *const SQL_QUERY_SELECT = (
	"SELECT * FROM " DB_TABLE " ORDER BY " TIME_FLD ";"
);

static const char *const SQL_QUERY_INSERT_INTO = (
	"INSERT INTO " DB_TABLE " (" TIME_FLD ", " RX_FLD ", " TX_FLD ") "
    "VALUES (%ld, %lld, %lld);"
);

static const char *const SQL_QUERY_SELECT_FROM = (
	"SELECT * FROM " DB_TABLE " WHERE " TIME_FLD " >= %ld;"
);

static const char *const SQL_QUERY_SELECT_TO = (
	"SELECT * FROM " DB_TABLE " WHERE " TIME_FLD " <= %ld;"
);

static const char *const SQL_QUERY_SELECT_FROM_TO = (
	"SELECT * FROM " DB_TABLE " WHERE " TIME_FLD " >= %ld AND "
	TIME_FLD "<= %ld;"
);


bool print_db_table(char *db_name, time_t from, time_t to)
{
	sqlite3 *db;
	char *err_msg = 0;
	char *sql;
	int rc = sqlite3_open(db_name, &db);
	if (rc != SQLITE_OK) {
		log_err("Cannot open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return false;
	}
	log_dbg("From: %ld", from);
	log_dbg("To: %ld", to);
	if ((0 != from) && (0 != to)){
		sql = sqlite3_mprintf(SQL_QUERY_SELECT_FROM_TO, from, to);
	}else if ((0 != from) && (0 == to)){
		sql = sqlite3_mprintf(SQL_QUERY_SELECT_FROM, from);
	}else if ((0 == from) && (0 != to)){
		log_dbg("Only to selected: %ld", to);
		sql = sqlite3_mprintf(SQL_QUERY_SELECT_TO, to);
	}else{
		sql = strdup(SQL_QUERY_SELECT);
	}
	log_dbg("Query: %s", sql);

	printf("\n");
	sqlite3_stmt *res;

	rc = sqlite3_prepare_v2(db, sql , -1,
		&res, 0);
	if (rc != SQLITE_OK ) {
		log_err("Failed to select data.");
        log_err("SQL error: %s", err_msg);
		sqlite3_free(err_msg);
		sqlite3_close(db);
		return false;
	}

	printf("|%20s | %16s | %16s|\n", "time", "rx", "tx");
	for (int i = 0; i<60; i++)
		printf("-");
	printf("\n");

	char timebuff[N_BUFF];
	while(sqlite3_step(res) == SQLITE_ROW)
	{
	time_t t = (time_t)sqlite3_column_int(res, 1);
	strftime(timebuff, N_BUFF, DATE_FORMAT, localtime(&t));


	printf("|%20s | %16lld | %16lld|\n",
		   timebuff,
		   sqlite3_column_text(res, 2),
		   sqlite3_column_text(res, 3));
	}
	sqlite3_close(db);

	return true;
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
