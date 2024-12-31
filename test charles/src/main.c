#include "utils.h"
#include "csvutils.h"
#include "race.h"
#include "state.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Usage :
//  - ./f1_sim            => reprendre la session en cours
//  - ./f1_sim NomCourse  => nouveau GP (classique)
//  - ./f1_sim NomCourse special => nouveau GP (sprint)

int main(int argc, char *argv[])
{
    // Charger la liste des circuits
    Circuit circuits[MAX_CIRCUITS];
    int nbC = load_circuits_from_csv("circuits.csv", circuits, MAX_CIRCUITS);
    if(nbC<1){
        printf("Impossible de charger circuits.csv\n");
        return 1;
    }

    GrandPrixWeekend gpw;
    memset(&gpw, 0, sizeof(gpw));

    if(argc==1){
        // Reprendre
        if(load_state(&gpw)<0){
            printf("Aucune session en cours (gp_state.dat introuvable)\n");
            return 0;
        }

        if(gpw.currentSession==SESS_FINISHED){
            printf("Le GP '%s' est déjà terminé.\n", gpw.gpName);
            return 0;
        }

        // Lancer la session courante
        run_session(&gpw);
        // Fin => points, etc.
        end_session(&gpw);

        // Passer à la session suivante
        next_session(&gpw);

        if(gpw.currentSession==SESS_FINISHED){
            // On sauvegarde le classement final
            printf("GP '%s' terminé !\n", gpw.gpName);
            save_final_classification(&gpw, "final_result.txt");
            remove_state();
            printf("Classement final écrit dans 'final_result.txt'.\n");
        } else {
            // Sauvegarder l’état
            save_state(&gpw);
            printf("Session terminée. Prochaine session: %d.\n", gpw.currentSession);
        }
    }
    else {
        // On a au moins 1 argument => on démarre un nouveau GP
        // ex: ./f1_sim "Bahrain" special
        int isSprint = 0;
        if(argc>=3 && strcmp(argv[2], "special")==0){
            isSprint=1;
        }

        init_new_gp(&gpw, argv[1], isSprint, circuits, nbC);

        // On commence direct par la session SESS_P1
        // Lancer la session
        run_session(&gpw);
        // Fin => points
        end_session(&gpw);

        // Prochaine session
        next_session(&gpw);

        if(gpw.currentSession==SESS_FINISHED){
            // Si c'était un mini-GP d'une seule session => improbable
            printf("GP '%s' terminé en 1 session ???\n", gpw.gpName);
            save_final_classification(&gpw, "final_result.txt");
            remove_state();
        } else {
            // On sauvegarde l’état
            save_state(&gpw);
            printf("Session initiale terminée. Prochaine session = %d\n", gpw.currentSession);
        }
    }

    return 0;
}
