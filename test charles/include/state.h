#ifndef STATE_H
#define STATE_H

#include "race.h"

// Sauvegarde / reprise de l'état complet du Grand Prix (GPState)
int load_state(GPState *gp);
void save_state(const GPState *gp);
void remove_state();

#endif /* STATE_H */
