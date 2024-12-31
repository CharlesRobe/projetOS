#include "utils.h"
#include "csvutils.h"
#include "race.h"
#include "state.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    // Charger CSV
    Circuit circuits[MAX_CIRCUITS];
    int nbC= load_circuits_from_csv("circuits.csv", circuits, MAX_CIRCUITS);
    if(nbC<1){
        printf("Erreur: impossible de charger circuits.csv\n");
        return 1;
    }

    GPState gp;

    if(argc==1){
        // reprise
        if(load_state(&gp)<0){
            printf("Aucune session en cours (gp_state.dat introuvable)\n");
            return 0;
        }
        if(gp.currentSession==SESS_FINISHED){
            printf("Le GP '%s' est déjà terminé.\n", gp.gpName);
            return 0;
        }

        run_session(&gp);
        end_session(&gp);
        next_session(&gp);

        if(gp.currentSession==SESS_FINISHED){
            printf("GP '%s' terminé!\n", gp.gpName);
            save_final_classification(&gp, "final_result.txt");
            remove_state();
        } else {
            save_state(&gp);
            printf("Session terminée => Prochaine session: %d\n",(int)gp.currentSession);
        }
    }
    else {
        // ./f1_sim <numeroCircuit> [special]
        int cNum= atoi(argv[1]);
        int isSprint=0;
        if(argc>=3 && strcmp(argv[2],"special")==0){
            isSprint=1;
        }
        init_new_gp(&gp, cNum, isSprint, circuits, nbC);

        run_session(&gp);
        end_session(&gp);
        next_session(&gp);

        if(gp.currentSession==SESS_FINISHED){
            printf("GP '%s' terminé en 1 session ???\n", gp.gpName);
            save_final_classification(&gp,"final_result.txt");
            remove_state();
        } else {
            save_state(&gp);
            printf("Session initiale terminée. Prochaine session=%d\n",(int)gp.currentSession);
        }
    }

    return 0;
}
