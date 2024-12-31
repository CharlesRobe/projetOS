#ifndef RACE_H
#define RACE_H

#include "f1shared.h"

void run_essai_1h(F1Shared *f1);
void run_qualifQ1(F1Shared *f1);
void run_qualifQ2(F1Shared *f1);
void run_qualifQ3(F1Shared *f1);
void run_course(F1Shared *f1);

void load_circuit_length(const char *filename, int circuitNum, F1Shared *f1);

#endif /* RACE_H */
