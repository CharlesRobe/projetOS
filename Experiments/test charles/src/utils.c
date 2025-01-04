#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

void error_exit(const char *msg)
{
    perror(msg);
    exit(1);
}
