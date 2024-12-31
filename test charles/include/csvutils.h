#ifndef CSVUTILS_H
#define CSVUTILS_H

#include "circuit.h"

// Charge les circuits depuis "circuits.csv" dans circuits[]
// Retourne le nombre de circuits lus, ou -1 si erreur
int load_circuits_from_csv(const char *filename, Circuit circuits[], int maxCirc);

#endif /* CSVUTILS_H */
