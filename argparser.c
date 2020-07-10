#define _XOPEN_SOURCE 700

#include "logging.h"

#include <stdio.h>
#include <time.h>
#include <argp.h>
#include <stdbool.h>

static const short ARG_COUNT = 9;

static const char *DATE_FORMAT = "%d %b %Y %H:%M:%S";

enum error {
    AE_OK = 0,          /* No error                                        */
    AE_NOARGS,          /* No arguments provided                           */
    AE_WRONGARG,        /* Argument has wrong format                       */
    AE_NENARGS,         /* Not enough arguments                            */
    AE_TMARGS,          /* Too much arguments                              */
    AE_CVGERR,          /* Convergence error                               */
	AE_MPERR,           /* Mode parser error                               */
    __AE_LAST           /* Last item. So that array sizes match everywhere */
};
//error: too few arguments to function ‘fprintf’
static const char *const error_msg[] = {
    [AE_OK]			= "",
    [AE_NOARGS]   	= "No arguments provided",
    [AE_WRONGARG] 	= "Wrong argument '%s'",
    [AE_NENARGS]  	= "Too few arguments",
    [AE_TMARGS]   	= "Too many arguments",
    [AE_CVGERR]   	= "Convergence unreachable",
	[AE_MPERR]	  	= "Wrong argument",		//!!!!!!!!!!
    [__AE_LAST]   = NULL
};

// net-collect -daemon -db=./stat.db -db=wlp2s0
// net-collect -stat -db=./stat.db
// add sort type
//				- by date
//				- by upload
//				- by download
//				- by total

/* Program documentation. */
static char doc[] =
  "firs\
options\
\vThis part of the documentation comes *after* the options;\
 note that the text is automatically filled, but it's possible\
 to force a line-break, e.g.";

//{"interface",   'i', "NAME", 	0                  	"Network interface name" }};
static struct argp_option options[] = {
    {"daemon",  	'd', 0,      	0,                  "Run in the daemon mode" },
    {"stat",  		's', 0,      	0,                  "Run in the statistics mode" },
    {"db",    		111, "PATH", 	0,                 	"Datebase path" },
    {"interface", 	'i', "NAME", 	0,                 	"Network interface name" },
	{0,0,0,0, 		"The following options should be grouped together in daemon mode:" },
    {"period",   	'p', "SECOND", 	OPTION_ARG_OPTIONAL, "The period of writing data " },
    {"rotete",   	'r', "MINUTE", 	OPTION_ARG_OPTIONAL, "Running time" },
    {"limit",   	'l', "MiB", 	OPTION_ARG_OPTIONAL, "Data limit" },
    {"cmd",   		'c', "STRING", 	OPTION_ARG_OPTIONAL, "Internal command" },
	{0,0,0,0,		"The following options should be grouped together in statistics mode:" },
    {"from",   		'f', "DATE",	OPTION_ARG_OPTIONAL, "From which date" },
    {"to",   		't', "DATE", 	OPTION_ARG_OPTIONAL, "Until which date" },
    { 0 }
};

// mode of run anction
enum run_mode{
    RM_DAEMON,
    RM_STAT
};

static const char *const run_mode_str[] = {
    [RM_DAEMON]	= "daemon",
    [RM_STAT]   = "stat"
};

struct arguments{
      char *dbfile;
      enum run_mode mode;
      union _data{
          struct dargs{
              long long period;
              long long rotate;
              char *netinterface;
              long long limMiB;
              char *commandstr;
          };
          time_t from;
          time_t to;
      };
};

static inline bool ld_conv(const char *str, long long *dstptr)
{
    char etc;
    return (sscanf(str, "%d%c", dstptr, &etc) == 1) && (0 < *dstptr);
}

static bool time_conv(const char *str, time_t *t)
{
	struct tm tm = {0};
	char *s = strptime(str, DATE_FORMAT, &tm);
	return (NULL != s) && (-1 != (*t = mktime(&tm)));
}

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	void report(enum error err)
    {
        fprintf(stderr, "Error: %s.\n", error_msg[err]);
        // exit(error_retcodes[err]);
    }

	struct arguments *args = state->input;
	switch (key){
	case 'd':
		if (args->mode == RM_STAT)
			fprintf(stderr, "Cannot work in two modes \n");//usage
		args->mode = RM_DAEMON;
		log_info("Daemon mode is selected");
		break;
	case 's':
		if (args->mode == RM_DAEMON)
			fprintf(stderr, "Cannot work in two modes \n");
		args->mode = RM_STAT;
		log_info("Statistic mode is selected");
		break;
	case 111:
		args->dbfile = arg;
		log_info("DataBase file %s", arg);
		break;
	case 'i':
		args->netinterface = arg;
		log_info("Network interface name %s", arg);
		break;
	case 'p':;
		long long period;
		 if (!ld_conv(arg, &period) || ((long long)period < 0))
			report(AE_CVGERR);//return
		args->period = period;
		log_info("Update interval %d[second]", period);
		break;
	case 'r':;
		long long rotate;
		if (!ld_conv(arg, &rotate) || ((long long)rotate < 0))
			report(AE_CVGERR);//return;
		args->rotate = rotate;
		log_info("Update rotete period %d[minute]", rotate);
		break;
	case 'l':;
		long long limMiB;
		if (!ld_conv(arg, &limMiB) || ((long long)limMiB < 0))
			report(AE_CVGERR);//return
		args->limMiB = limMiB;
		log_info("Limit %d[MiB]", limMiB);
		break;
	case 'c':
		args->commandstr = arg;
		log_info("Internal command %s", arg);
		break;
	case 'f':;
		time_t from;
		if (!time_conv(arg, &from))
			report(AE_CVGERR);//return
		args->from = from;
		log_info("from date %ld", from);
	case 't':;
		time_t to;
		if (!time_conv(arg, &to))
			report(AE_CVGERR);//return
		log_info("to date %ld", to);
		args->to = to;
	case ARGP_KEY_NO_ARGS:
		log_info("ARGP_KEY_NO_ARGS %ld", ARGP_KEY_NO_ARGS);
		argp_usage (state);
	case ARGP_KEY_ARG:
		if (state->arg_num >= ARG_COUNT){
			log_info("ARGP_KEY_ARG %ld", ARGP_KEY_ARG);
			argp_usage(state); // Too meny arguments
		}
		break;
	case ARGP_KEY_END:
		if (state->arg_num <= 2){
			log_info("Arg num %ld", state->arg_num );
			argp_usage(state); // Not enough arguments
		}
		break;
	default:
		log_info("ARGP_ERR_UNKNOWN %ld", ARGP_ERR_UNKNOWN);
		return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

static char args_doc[] = "MODE [daemon] \nMODE [stat]";

static struct argp argp = { options, parse_opt, args_doc, doc};



int main(int argc, char *argv[])
{
	/*default values*/
	struct arguments arguments = {.period = 10, .rotate = 60, .from = 0, .to = 0};

	argp_parse (&argp, argc, argv, 0, 0, &arguments);

	return 0;
}
