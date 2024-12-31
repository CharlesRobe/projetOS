#ifndef CIRCUIT_H
#define CIRCUIT_H

#define MAX_CIRCUITS 30

typedef struct {
    int   numeroCourse;    // 1,2,3,...
    double longueur;       // ex: 5.412 km
    char  pays[64];        // ex: "Bahrain"
    char  nom[128];        // ex: "Bahrain International Circuit"
} Circuit;

#endif /* CIRCUIT_H */
