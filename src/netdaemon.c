#include "logging.h"
#include "argparser.h"
#include "netdaemon.h"
#include "collectdb.h"

#include <sqlite3.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#define BUFSIZE 128
#define DAEMON_NAME "net-collect"

static const char *RXstr = "RX:";
static const char *TXstr = "TX:";
static const char *CMDIPLINK = "ip -s link ls dev ";

enum error {
    ND_OK = 0,      /* No error                                         */
    ND_MAERR,       /* Error memory allocate                            */
    ND_CNERR,       /* Command not found                                */
    ND_PCERR,       /*Parse out error                                   */
    ND_OPERR,       /*Error opening pipe                                 */
    __ND_LAST       /* Last item. So that array sizes match everywhere  */
};
struct netdata{
    long long TX;
    long long RX;
    time_t now;
};

char ** read_prog_out(char *cmd)
{
    char buf[BUFSIZE];
    FILE *fp;

    printf("cmd>> %s\n", cmd);

    if ((fp = popen(cmd, "r")) == NULL) {
        printf("Error opening pipe!\n");
        return NULL;
    }
    char **strs = (char **) malloc(sizeof(char *));
    if (strs == NULL) {
        perror("malloc failed");
        exit(1);
    }
    int cnt = 0;
    while (NULL != fgets(buf, BUFSIZE, fp)) {
        if (cnt > 0){
            strs = (char **) realloc(strs, sizeof(char *) * (cnt + 1));
            if (NULL == strs) {
                perror("realloc failed");
                exit(1);
            }
        }
        strs[cnt] = strdup(buf);
        //printf("OUTPUT: %s", strs[cnt]);
        cnt++;
    }
    if(pclose(fp))  {
        printf("Command not found or exited with error status\n");
        return NULL;
    }

    return  strs;
}

bool parce_cmd_out(char **outcmd, struct netdata *data)
{
    char *delim = " ";
    char *strtoken;
    int cnt = 0;
    while (NULL != outcmd[cnt]){
        strtoken = strtok(outcmd[cnt], delim);
        if (!strcmp(strtoken, RXstr)){
            cnt++;
            // (NULL != outcmd[cnt])
            strtoken = strtok(outcmd[cnt], delim);
            data->RX = atoi(strtoken);
            continue;
        }
        if (!strcmp(strtoken, TXstr)){
            cnt++;
            // (NULL != outcmd[cnt])
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
    pid_t pid = 0, sid =0;

    pid = fork();

    if (pid < 0)
        return false;

    if (pid > 0)
    {
        //printf("Net-collect daemon start. Process id: %d \n", pid);
        // return success
        exit(EXIT_SUCCESS);
    }
    umask(0);
    // chdir("/");

    // Close stdin. stdout and stderr
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

    if (outstr == NULL){
        // fprintf(fp, "Alloc error\n"); // print to log
        return false;
    }

    if (!parce_cmd_out(outstr, data)){
        // fprintf(fp, "Parce error\n"); // print to log
        return false;
        goto free_mem;
    }
    ret = true;

free_mem:

    while(NULL == outstr[cnt]){
        free(outstr[cnt]);
        cnt++;
    }
    free(outstr);
    return ret;
}

// int main(void)
bool daemod_run(struct arguments *args)
{
    if (access(args->dbfile, F_OK) == -1){
        fprintf(stderr, "File: %s does not exist!\n", args->dbfile);
        exit(1);
    }
    // char *iname = "wlp3s0";
    int cmdlen = strlen(CMDIPLINK) + strlen(args->netinterface) + 1;
    char *cmd = (char*) calloc(cmdlen, sizeof(cmd));
    strcpy(cmd, CMDIPLINK);
    strcat(cmd, args->netinterface);

    // printf("cmd>> %s\n", cmd);

    struct netdata netdata={-1, -1, time(NULL)};
    log_info("Command %s.", cmd);
    long long ntick = 10;
    log_info("N tick %lld.", ntick);
    long long cnt = 0;

    char *msg;
    sqlite3 *db;

    log_info("Connection to %s is success.", args->dbfile);
    init_daemon();
    FILE *fp = NULL;
    fp = fopen ("daemonlog.log", "w+");
    while (1)
    {
        cnt++;
        if (!collect_stat(cmd, &netdata)){
            fprintf(fp, "Failture collect_stat \n");
            exit(1);
        }
        fprintf(fp, "%d: Time: %d \t RX: %lld \t  TX: %lld \n",
            cnt, netdata.now, netdata.RX, netdata.TX);

        if (!write_to_db(db, args->dbfile, netdata.now, netdata.RX, netdata.TX, &msg)){
            fprintf(fp, "Error: %s\n", msg);
            sqlite3_close(db);
            exit(1);
        }else{
            fprintf(fp, "Write to db is success.\n");
        }
        sleep(5);
        if (cnt >= ntick)
            break;
        fflush(fp);

    }
    sqlite3_close(db);
    fprintf(fp, "DB connection is closed.\n");
    fprintf(fp, "Daemon terminated.");
    fclose(fp);
    return true;
}
