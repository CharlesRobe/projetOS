#ifndef CSVUTILS_H
#define CSVUTILS_H

#include "circuit.h"

// Lit le fichier CSV "circuits.csv" et stocke le résultat dans circuits[].
// Renvoie le nombre de circuits lus (22 dans votre cas), ou -1 en cas d’erreur.
int load_circuits_from_csv(const char *filename, Circuit circuits[], int maxCircuits);

#endif /* CSVUTILS_H */
