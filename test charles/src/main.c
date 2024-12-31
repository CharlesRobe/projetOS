#include "utils.h"
#include "csvutils.h"
#include "race.h"
#include <stdio.h>
#include <stdlib.h>

// On va boucler sur tous les GPs (22) : P1->P2->P3->Q1->Q2->Q3->(SPRINT)->RACE
// puis passer au GP suivant.

int main(void)
{
    // 1) Charger la liste des circuits depuis circuits.csv
    Circuit circuits[MAX_CIRCUITS];
    int nbC = load_circuits_from_csv("circuits.csv", circuits, MAX_CIRCUITS);
    if (nbC <= 0) {
        printf("Erreur: impossible de charger circuits.csv\n");
        return 1;
    }
    printf("Chargé %d circuits depuis circuits.csv\n", nbC);

    // 2) Initialiser le championnat
    Championship champ;
    init_championship(&champ, circuits, nbC);

    // 3) Pour chaque GP, on fait la séquence (P1->P2->P3->Q1->Q2->Q3->(SPRINT)->RACE)
    for (int gpIndex = 0; gpIndex < champ.nbGP; gpIndex++) {
        champ.currentGP = gpIndex;
        printf("\n\n=== Grand Prix #%d : %s (%s) ===\n",
               gpIndex+1,
               champ.gpList[gpIndex].circuit.nom,
               champ.gpList[gpIndex].circuit.pays);

        // On remet STAGE_NONE avant de démarrer
        champ.currentStage = STAGE_NONE;

        // Séquence
        next_stage(&champ); // P1
        run_current_session(&champ);
        end_stage(&champ, champ.currentStage);

        next_stage(&champ); // P2
        run_current_session(&champ);
        end_stage(&champ, champ.currentStage);

        next_stage(&champ); // P3
        run_current_session(&champ);
        end_stage(&champ, champ.currentStage);

        next_stage(&champ); // Q1
        run_current_session(&champ);
        end_stage(&champ, champ.currentStage);

        next_stage(&champ); // Q2
        run_current_session(&champ);
        end_stage(&champ, champ.currentStage);

        next_stage(&champ); // Q3
        run_current_session(&champ);
        end_stage(&champ, champ.currentStage);

        // Sprint ou Race
        if (champ.gpList[gpIndex].isSpecial == 1) {
            // SPRINT
            next_stage(&champ); // SPRINT
            run_current_session(&champ);
            end_stage(&champ, champ.currentStage);

            // RACE
            next_stage(&champ);
            run_current_session(&champ);
            end_stage(&champ, champ.currentStage);
        } else {
            // Direct Race
            next_stage(&champ); // RACE
            run_current_session(&champ);
            end_stage(&champ, champ.currentStage);
        }

        printf("=== Fin du GP #%d ===\n", gpIndex+1);
    }

    // 4) Saison terminée => on sauvegarde le classement final
    save_championship(&champ, "championship_final.csv");

    printf("\n>>> Saison terminée ! Classement final dans 'championship_final.csv'.\n");
    return 0;
}
