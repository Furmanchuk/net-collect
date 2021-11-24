/*
 * Module for working with the SQLite database.
 *
 * (C) 2020 Vadym Furmanchuk
 *
 * Homepage: https://github.com/Furmanchuk/net-collect/
 *
 * License:
 * The MIT License (MIT)
 */
//test
#include <stdio.h>

#include "collectdb.h"
#include "logging.h"

#include <sqlite3.h>
#include <stdbool.h>

#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DB_TABLE "netcollect"
#define ID_FLD "id"
#define TIME_FLD "time"
#define RX_FLD "rx"
#define TX_FLD "tx"

#define N_BUFF 20

static const char *DATE_FORMAT = "%Y %m %d %H:%M:%S";

static const char *const SQL_QUERY_CREATE_TABLE =
    ("CREATE TABLE " DB_TABLE " (" ID_FLD " INTEGER PRIMARY KEY NOT NULL,"
    TIME_FLD " INTEGER," RX_FLD " INTEGER ," TX_FLD " INTEGER );");

static const char *const SQL_QUERY_SELECT =
    ("SELECT * FROM " DB_TABLE " ORDER BY " TIME_FLD ";");

static const char *const SQL_QUERY_INSERT_INTO =
    ("INSERT INTO " DB_TABLE " (" TIME_FLD ", " RX_FLD ", " TX_FLD ") "
     "VALUES (%ld, %lld, %lld);");

static const char *const SQL_QUERY_SELECT_FROM =
    ("SELECT * FROM " DB_TABLE " WHERE " TIME_FLD " >= %ld;");

static const char *const SQL_QUERY_SELECT_TO =
    ("SELECT * FROM " DB_TABLE " WHERE " TIME_FLD " <= %ld;");

static const char *const SQL_QUERY_SELECT_FROM_TO =
    ("SELECT * FROM " DB_TABLE " WHERE " TIME_FLD " >= %ld AND " TIME_FLD
     "<= %ld;");

 static const char *const error_msg[] = {[CE_OK] = "",
                                         [CE_OE] = "Can't open DataBase",
                                         [CE_EE] = "FFailed to execute SQL statement",
                                         [CE_SE] = "Failed to select SQL statement",
                                         [__CE_LAST] = NULL};

inline void print_line(int len)
{
    for(int i = 0; i < len; i++)
        printf("-");
    printf("\n");
}

enum cerror print_db_table(char *db_name, time_t from, time_t to)
{
    sqlite3 *db;
    char *sql;
    int rc = sqlite3_open(db_name, &db);
    if (rc != SQLITE_OK) {
        log_err(" %s\n SQL error : %s\n", error_msg[CE_OE], sqlite3_errmsg(db));
        sqlite3_close(db);
        return CE_OE;
    }

    log_dbg("From: %ld", from);
    log_dbg("To: %ld", to);
    if ((0 != from) && (0 != to)) {
        sql = sqlite3_mprintf(SQL_QUERY_SELECT_FROM_TO, from, to);
    } else if ((0 != from) && (0 == to)) {
        sql = sqlite3_mprintf(SQL_QUERY_SELECT_FROM, from);
    } else if ((0 == from) && (0 != to)) {
    log_dbg("Only to selected: %ld", to);
        sql = sqlite3_mprintf(SQL_QUERY_SELECT_TO, to);
    } else {
        sql = strdup(SQL_QUERY_SELECT);
    }
    log_dbg("Query: %s", sql);

    sqlite3_stmt *res;

    printf("|%20s |  %16s| %16s|\n", "time", "RX(byte)", "TX(byte)");
    print_line(60);

    rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
    if (rc != SQLITE_OK) {
        log_err("%s", error_msg[CE_SE]);
        sqlite3_close(db);
        return CE_SE;
    }

    int totalRX = 0, totalTX = 0;
    long long rx, tx;
    char timebuff[N_BUFF];
    while (sqlite3_step(res) == SQLITE_ROW) {
        time_t t = (time_t)sqlite3_column_int(res, 1);
        strftime(timebuff, N_BUFF, DATE_FORMAT, localtime(&t));
        rx = sqlite3_column_int(res, 2);
        tx = sqlite3_column_int(res, 3);
        totalRX += rx;
        totalTX += tx;
        printf("|%20s | %16lld | %16lld|\n", timebuff, rx, tx);
    }
    long double MiB = 9.53674E-7;
    printf("Total RX %10Lf [MiB]\n", totalRX * MiB);
    printf("Total TX %10Lf [MiB]\n", totalTX * MiB);
    printf("Total    %10Lf [MiB]\n", (totalRX + totalTX) * MiB);

    sqlite3_close(db);

    return CE_OK;
}

enum cerror write_to_db(sqlite3 *db, char *db_name, time_t _time, long long rx,
                 long long tx, char **msg)
{
    int rc;
    char *sql = sqlite3_mprintf(SQL_QUERY_INSERT_INTO, _time, rx, tx);
    char *err_msg;

    rc = sqlite3_open(db_name, &db);
    if (rc != SQLITE_OK) {
        *msg = strdup(sqlite3_errmsg(db));
        return CE_OE;
    }

    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        *msg = strdup(err_msg);
        return CE_EE;
    }
    return CE_OK;
}

enum cerror create_db(char *db_name, char **err_msg)
{
    log_dbg("Database path %s", db_name);
    sqlite3 *db;
    char *e_msg;
    if (SQLITE_OK != (sqlite3_open(db_name, &db))){
        *err_msg = strdup(sqlite3_errmsg(db));
        log_err("% s \nSQL error: ", error_msg[CE_OE], err_msg);
        sqlite3_close(db);
        return CE_OE;
    }
    const char *sql = SQL_QUERY_CREATE_TABLE;
    log_dbg("SQL query: %s", sql);
	/* Execute SQL statement */
    int rc = sqlite3_exec(db, sql, 0, 0, &e_msg);
	if(SQLITE_OK != rc){
        log_err("%s \nSQL error: ", error_msg[CE_EE], e_msg);
        return CE_EE;
    }
    sqlite3_close(db);
    return CE_OK;
}
