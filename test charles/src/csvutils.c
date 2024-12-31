#include "csvutils.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int load_circuits_from_csv(const char *filename, Circuit circuits[], int maxCircuits)
{
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("fopen");
        return -1;
    }

    char line[256];
    int count = 0;

    // Lire la première ligne (header) et l’ignorer
    if (fgets(line, sizeof(line), f) == NULL) {
        fclose(f);
        return -1;
    }

    while (fgets(line, sizeof(line), f) != NULL) {
        if (count >= maxCircuits) break; 

        // Format CSV : numéro, taille, pays, nom
        // On utilise strtok()
        char *token = strtok(line, ",");
        if (!token) continue;
        circuits[count].numeroCourse = atoi(token);

        token = strtok(NULL, ",");
        if (!token) continue;
        circuits[count].longueur = atof(token);

        token = strtok(NULL, ",");
        if (!token) continue;
        strncpy(circuits[count].pays, token, sizeof(circuits[count].pays));
        circuits[count].pays[sizeof(circuits[count].pays)-1] = '\0';

        token = strtok(NULL, "\n");
        if (!token) continue;
        strncpy(circuits[count].nom, token, sizeof(circuits[count].nom));
        circuits[count].nom[sizeof(circuits[count].nom)-1] = '\0';

        count++;
    }

    fclose(f);
    return count;
}
