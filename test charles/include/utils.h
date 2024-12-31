#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <signal.h>

void error_exit(const char *msg);

// Petite fonction pour faire un affichage ASCII "beau tableau"
void print_ascii_table_header(const char *title);
void print_ascii_table_row(int pos, int carNum, double bestLap, int points);
void print_ascii_table_footer();

#endif /* UTILS_H */
