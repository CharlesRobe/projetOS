#include "utils.h"
#include <stdio.h>

void error_exit(const char *msg)
{
    perror(msg);
    _exit(1);
}

// On fait un petit tableau ASCII
void print_ascii_table_header(const char *title)
{
    printf("  +---------------------------------------------------------+\n");
    if(title && title[0]!='\0'){
        printf("  | %-53s |\n", title);
        printf("  +---------------------------------------------------------+\n");
    }
    printf("  | POS | CAR# |   BESTLAP  | POINTS                       |\n");
    printf("  +-----+------+------------+------------------------------+\n");
}

void print_ascii_table_row(int pos, int carNum, double bestLap, int points)
{
    printf("  | %3d | %4d | %10.3f | %-28d|\n",
           pos, carNum, bestLap, points);
}

void print_ascii_table_footer()
{
    printf("  +---------------------------------------------------------+\n");
}
