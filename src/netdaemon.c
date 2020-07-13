#include "logging.h"
#include "argparser.h"
#include "netdaemon.h"

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
    ND_OPERR,       /*Error opening pipe                                   */
    __ND_LAST       /* Last item. So that array sizes match everywhere  */
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
    char **strs = (char **) malloc(sizeof(char *) * 1);
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
            log_dbg("RX: %lld", data->RX);
            continue;
        }
        if (!strcmp(strtoken, TXstr)){
            cnt++;
            // (NULL != outcmd[cnt])
            strtoken = strtok(outcmd[cnt], delim);
            data->TX = atoi(strtoken);
            log_dbg("TX: %lld", data->TX);
            // printf("TX: %lld\n", data->TX);
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
    // Open a log file in write mode.

    return true;
}

bool collect_stat(char *command, struct netdata *data)
{
    // void free_mem(char **arr)
    // {
    //     int cnt = 0;
    //     while(NULL == arr[cnt]){
    //         free(arr[cnt]);
    //     }
    //     free(arr);
    // }

    char **outstr = read_prog_out(command);

    if (outstr == NULL){
        fprintf(stderr, "%s\n", "Alloc error"); // print to log
        return false;
    }

    if (!parce_cmd_out(outstr, data)){
        fprintf(stderr, "%s\n", "Parce error"); // print to log
        // free_mem(outstr);
        return false;
    }

    // free_mem(outstr);
    return true;
}

// int main(void)
bool daemod_run(struct arguments *args)
{

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

        char *str = ctime(&netdata.now);
        str[strlen(str)-1] = '\0';
        fprintf(fp, "%d: Time: %s \t RX: %lld \t  TX: %lld \n", cnt, str, netdata.RX, netdata.TX);


        // fprintf(fp, "Log ...\n");
        //Dont block context switches, let the process sleep for some time
        sleep(1);
        fflush(fp);
        // Implement and call some function that does core work for this daemon.
        if (cnt >= ntick)
            break;

    }
    fprintf(fp, "Daemon terminated.");
    fclose(fp);
    return true;
    // return 0;
}
