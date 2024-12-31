#ifndef RWCOURTOIS_H
#define RWCOURTOIS_H

extern int nblect;
extern int mutex;
extern int mutlect;

void rw_init();
void rw_cleanup();

void lire_debut();
void lire_fin();
void ecrire_debut();
void ecrire_fin();

#endif /* RWCOURTOIS_H */
