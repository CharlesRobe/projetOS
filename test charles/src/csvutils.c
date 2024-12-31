#include "csvutils.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int load_circuits_from_csv(const char *filename, Circuit circuits[], int maxCirc)
{
    FILE *f = fopen(filename, "r");
    if(!f) {
        perror("fopen");
        return -1;
    }

    char line[256];
    // Ignorer header
    if(!fgets(line, sizeof(line), f)) {
        fclose(f);
        return -1;
    }

    int count=0;
    while(fgets(line, sizeof(line), f)) {
        if(count >= maxCirc) break;

        // Format : numero, longueur, pays, nom
        // On découpe
        char *token = strtok(line, ",");
        if(!token) continue;
        circuits[count].numeroCourse = atoi(token);

        token = strtok(NULL, ",");
        if(!token) continue;
        circuits[count].longueur = atof(token);

        token = strtok(NULL, ",");
        if(!token) continue;
        strncpy(circuits[count].pays, token, sizeof(circuits[count].pays));
        circuits[count].pays[sizeof(circuits[count].pays)-1] = '\0';

        token = strtok(NULL, "\n");
        if(!token) continue;
        strncpy(circuits[count].nom, token, sizeof(circuits[count].nom));
        circuits[count].nom[sizeof(circuits[count].nom)-1] = '\0';

        count++;
    }

    fclose(f);
    return count;
}
