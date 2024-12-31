#include "utils.h"

void error_exit(const char *msg)
{
    perror(msg);
    _exit(1);
}
