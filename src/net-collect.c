#include "argparser.h"

#include <stdio.h>

int main(int argc, char *argv[])
{

    struct arguments arguments;

    if (!args_parce(argc, argv, &arguments))
        fprintf(stderr, "%s\n", "Error: Pasing failture");



    return 0;
}
