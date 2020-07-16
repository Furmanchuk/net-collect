#include "collectdb.h"
#include "logging.h"
#include "netdaemon.h"

#include <sqlite3.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define BUFSIZE 128
#define DAEMON_NAME "net-collect"

static long double MiB = 9.53674E-7;

static const char *RXstr = "RX:";
static const char *TXstr = "TX:";
static const char *CMDIPLINK = "ip -s link ls dev ";

struct netdata {
    long long TX;
    long long RX;
    time_t now;
};

char **read_prog_out(char *cmd)
{
    char buf[BUFSIZE];
    FILE *fp;

    if ((fp = popen(cmd, "r")) == NULL) {
        return NULL;
    }
    char **strs = (char **)malloc(sizeof(char *));
    if (strs == NULL) {
        exit(1);
    }
    int cnt = 0;
    while (NULL != fgets(buf, BUFSIZE, fp)) {
        if (cnt > 0) {
            strs = (char **)realloc(strs, sizeof(char *) * (cnt + 1));
            if (NULL == strs) {
                exit(1);
            }
        }
        strs[cnt] = strdup(buf);
        cnt++;
    }
    if (pclose(fp))
        return NULL;

  return strs;
}

bool parce_cmd_out(char **outcmd, struct netdata *data)
{
    char *delim = " ";
    char *strtoken;
    int cnt = 0;
    while (NULL != outcmd[cnt]) {
        strtoken = strtok(outcmd[cnt], delim);
        if (!strcmp(strtoken, RXstr)) {
            cnt++;
            strtoken = strtok(outcmd[cnt], delim);
            data->RX = atoi(strtoken);
            continue;
        }
        if (!strcmp(strtoken, TXstr)) {
            cnt++;
            strtoken = strtok(outcmd[cnt], delim);
            data->TX = atoi(strtoken);
            continue;
        }
    cnt++;
    }

    data->now = time(NULL);

    return ((data->TX > 0) || (data->RX > 0));
}

bool init_daemon(void)
{
    pid_t pid = 0, sid = 0;
    pid = fork();
    if (pid < 0)
        return false;

    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }
    umask(0);

  /* Close stdin, stdout and stderr */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    return true;
}

bool collect_stat(char *command, struct netdata *data)
{
    bool ret = false;
    int cnt = 0;
    char **outstr = read_prog_out(command);

  if (outstr == NULL) {
    return false;
  }

    if (!parce_cmd_out(outstr, data)) {
        return false;
        goto free_mem;
    }
    ret = true;

free_mem:

    while (NULL == outstr[cnt]) {
        free(outstr[cnt]);
        cnt++;
    }
    free(outstr);
    return ret;
}

void do_internal_cmd(char *cmd)
{
    if (fork() == 0) {
        system(cmd);
        exit(0);
    }
}

void daemod_run(char *dbpath, struct dargs *dargs)
{
    int cmdlen = strlen(CMDIPLINK) + strlen(dargs->netinterface) + 1;
    char *cmd = (char *)calloc(cmdlen, sizeof(cmd));
    strcpy(cmd, CMDIPLINK);
    strcat(cmd, dargs->netinterface);

    struct netdata netdata = {-1, -1, time(NULL)};
    log_info("Command %s.", cmd);
    /* Convert rotate to minute*/
    long long ntick = (dargs->rotate * 60) / dargs->period;
    log_info("N tick %lld.", ntick);
    long long cnt = 0;

    char *msg;
    sqlite3 *db;

    struct netdata buffdata[2];
    buffdata[0] = netdata;
    long long sumMiB = 0;
    bool limflag = false;

    log_info("cmd>> %s", cmd);
    init_daemon();
    FILE *fp = NULL;
    fp = fopen("daemonlog.log", "w+");
    while (1) {
    cnt++;
    if (!collect_stat(cmd, &netdata)) {
      fprintf(fp, "Failture collect_stat \n");
        exit(1);
    }
    buffdata[1] = buffdata[0];
    buffdata[0] = netdata;

    if (1 == cnt) {
        netdata.RX = 0;
        netdata.TX = 0;
    } else {
        netdata.RX = buffdata[0].RX - buffdata[1].RX;
        netdata.TX = buffdata[0].TX - buffdata[1].TX;
        sumMiB += (netdata.RX + netdata.TX); // * MiB;
        fprintf(fp, "sumMiB: %lld \n", sumMiB);
    }

    if (!limflag && (sumMiB * MiB >= dargs->limMiB) &&
        (NULL != dargs->commandstr)) {
        do_internal_cmd(dargs->commandstr);
        limflag = !limflag;
    }

    fprintf(fp, "%d: Time: %d \t RX: %lld \t  TX: %lld \n", cnt, netdata.now,
            netdata.RX, netdata.TX);

    if (!write_to_db(db, dbpath, netdata.now, netdata.RX, netdata.TX,
                     &msg)) {
        fprintf(fp, "Error: %s\n", msg);
        sqlite3_close(db);
        exit(1);
    } else {
        fprintf(fp, "Write to db is success.\n");
    }
        sleep(dargs->period);
        if (cnt >= ntick)
          break;
        fflush(fp);
    }
    sqlite3_close(db);
    fprintf(fp, "DB connection is closed.\n");
    fprintf(fp, "Daemon terminated.");
    fclose(fp);
}
