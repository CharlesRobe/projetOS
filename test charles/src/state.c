#include "state.h"
#include "utils.h"
#include <stdio.h>

// Fichier binaire
#define STATE_FILE "gp_state.dat"

int load_state(GrandPrixWeekend *gpw)
{
    FILE *f = fopen(STATE_FILE, "rb");
    if(!f) return -1;

    size_t sz = fread(gpw, sizeof(GrandPrixWeekend), 1, f);
    fclose(f);
    if(sz < 1) return -1;
    return 0;
}

void save_state(const GrandPrixWeekend *gpw)
{
    FILE *f = fopen(STATE_FILE, "wb");
    if(!f) {
        perror("fopen");
        return;
    }
    fwrite(gpw, sizeof(GrandPrixWeekend), 1, f);
    fclose(f);
}

void remove_state()
{
    remove(STATE_FILE);
}
