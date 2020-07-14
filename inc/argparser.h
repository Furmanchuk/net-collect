#include <stdbool.h>
#include <time.h>

enum run_mode{
    RM_DAEMON,
    RM_STAT
};
//!!!!!!!!!!!!
#pragma once
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

bool args_parce(int argc, char *argv[], struct arguments *args);
