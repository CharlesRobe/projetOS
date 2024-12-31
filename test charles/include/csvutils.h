#ifndef CSVUTILS_H
#define CSVUTILS_H

#include "circuit.h"

int load_circuits_from_csv(const char *filename, Circuit circuits[], int maxCirc);

#endif /* CSVUTILS_H */
