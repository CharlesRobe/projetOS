#ifndef CIRCUIT_H
#define CIRCUIT_H

#define MAX_CIRCUITS 30

typedef struct {
    int    numeroCourse;  // 1..24
    double longueur;      // ex. 5.412 (km)
    char   pays[64];
    char   nom[128];
} Circuit;

#endif /* CIRCUIT_H */
