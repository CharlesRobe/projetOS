#ifndef CIRCUIT_H
#define CIRCUIT_H

#define MAX_CIRCUITS 30

typedef struct {
    int    numeroCourse;
    double longueur;
    char   pays[64];
    char   nom[128];
} Circuit;

#endif /* CIRCUIT_H */
