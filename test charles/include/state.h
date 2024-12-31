#ifndef STATE_H
#define STATE_H

#include "race.h"

// Charge l’état depuis gp_state.dat
// Renvoie 0 si OK, -1 sinon
int load_state(GrandPrixWeekend *gpw);

// Sauvegarde l’état dans gp_state.dat
void save_state(const GrandPrixWeekend *gpw);

// Efface l’état => signale qu’on a fini
void remove_state();

#endif /* STATE_H */
