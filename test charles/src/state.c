#include "state.h"
#include <stdio.h>
#include <string.h>

#define STATE_FILE "gp_state.dat"

int load_state(GPState *gp)
{
    FILE *f = fopen(STATE_FILE, "rb");
    if(!f) return -1;
    size_t sz= fread(gp, sizeof(GPState),1,f);
    fclose(f);
    if(sz<1) return -1;
    return 0;
}

void save_state(const GPState *gp)
{
    FILE *f= fopen(STATE_FILE,"wb");
    if(!f) return;
    fwrite(gp,sizeof(GPState),1,f);
    fclose(f);
}

void remove_state()
{
    remove(STATE_FILE);
}
