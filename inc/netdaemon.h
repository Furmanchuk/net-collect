
#include "argparser.h"
#include <stdbool.h>
#pragma once
struct netdata{
    long long TX;
    long long RX;
    time_t now;
};

bool daemod_run(struct arguments *args);

// bool daemod_run(struct netdata *args);
