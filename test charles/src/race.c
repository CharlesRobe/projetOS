#include "race.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>

// Table des 20 numéros de voiture (fixe)
static int carNumbers[MAX_CARS] = {
    1, 11, 44, 63, 16, 55, 4, 81, 14, 18,
    10, 31, 23, 2, 22, 3, 77, 24, 20, 27
};

// Rechercher un circuit dont le nom contient gpName.
// Si rien ne match, on prend circuits[0].
void init_new_gp(GPState *gp,
                 const char *gpName,
                 int isSprint,
                 Circuit circuits[],
                 int nbCirc)
{
    memset(gp, 0, sizeof(GPState));
    gp->isSprint = isSprint;
    gp->currentSession = SESS_P1;  // On commence par P1
    strncpy(gp->gpName, gpName, sizeof(gp->gpName));
    gp->gpName[sizeof(gp->gpName)-1] = '\0';

    // Recherche dans circuits[]
    int found = -1;
    for(int i=0; i<nbCirc; i++){
        if(strstr(circuits[i].nom, gpName)!=NULL){
            found = i;
            break;
        }
    }
    if(found<0){
        // Pas trouvé => on prend le premier
        gp->circuit = circuits[0];
    } else {
        gp->circuit = circuits[found];
    }

    // Init voitures
    for(int i=0; i<MAX_CARS; i++){
        gp->cars[i].pid = 0;
        gp->cars[i].carNumber = carNumbers[i];
        gp->cars[i].status = CAR_RUNNING;
        gp->cars[i].bestLap = 9999.0;
        gp->cars[i].points = 0;
    }
}

// Lance la session courante
void run_session(GPState *gp)
{
    if(gp->currentSession==SESS_NONE || gp->currentSession==SESS_FINISHED) {
        printf("Aucune session à lancer.\n");
        return;
    }

    printf("\n=== [GP: %s | Session: %d] ===\n", gp->gpName, gp->currentSession);

    // 1) Créer un pipe par voiture
    int pipefd[MAX_CARS][2];
    for(int i=0; i<MAX_CARS; i++){
        if(pipe(pipefd[i])<0){
            error_exit("pipe");
        }
    }

    // 2) Fork 20 voitures
    for(int i=0; i<MAX_CARS; i++){
        pid_t p = fork();
        if(p<0){
            error_exit("fork");
        }
        else if(p==0){
            // Enfant
            close(pipefd[i][0]);  // on ferme la lecture
            run_car_process(i, pipefd[i][1]); 
        }
        else {
            // Parent
            gp->cars[i].pid = p;
            close(pipefd[i][1]);  // on ferme l’écriture
        }
    }

    // 3) Durée simulée de la session (en secondes)
    int duration=8;
    switch(gp->currentSession){
        case SESS_Q1: case SESS_Q2: case SESS_Q3:
            duration=6;
            break;
        case SESS_SPRINT:
            duration=5;
            break;
        case SESS_RACE:
            duration=10;
            break;
        default:
            duration=8;
    }

    // 4) On va rafraîchir l’affichage toutes les 2 secondes
    int steps = duration * (1000 / 2000); // ex.: 8 s / 2 s = 4
    for(int s=0; s<steps; s++){
        // Lire en non-bloquant sur chaque pipe
        for(int i=0; i<MAX_CARS; i++){
            double lapTime;
            int r = read(pipefd[i][0], &lapTime, sizeof(double));
            while(r>0){
                if(lapTime < gp->cars[i].bestLap){
                    gp->cars[i].bestLap = lapTime;
                }
                r = read(pipefd[i][0], &lapTime, sizeof(double));
            }
        }

        // Tri
        sort_cars_by_bestlap(gp);

        // Affichage "live"
        printf("\n--- [Live scoreboard: session %d, step=%d] ---\n", 
               gp->currentSession, s);
        display_classification(gp, "Classement provisoire", 1);

        sleep(2);
    }

    // 5) Fin de session => kill tous les enfants
    for(int i=0; i<MAX_CARS; i++){
        if(gp->cars[i].pid>0){
            kill(gp->cars[i].pid, SIGTERM);
        }
    }
    // Wait
    for(int i=0; i<MAX_CARS; i++){
        if(gp->cars[i].pid>0){
            waitpid(gp->cars[i].pid, NULL, 0);
            gp->cars[i].pid = 0;
        }
    }

    // 6) Lecture finale (récupérer derniers tours en buffer)
    for(int i=0; i<MAX_CARS; i++){
        double lapTime;
        while(read(pipefd[i][0], &lapTime, sizeof(double))>0){
            if(lapTime < gp->cars[i].bestLap){
                gp->cars[i].bestLap = lapTime;
            }
        }
        close(pipefd[i][0]);
    }

    // Tri final
    sort_cars_by_bestlap(gp);

    // 7) Affichage final de la session
    printf("\n=== Classement final de la session %d ===\n", gp->currentSession);
    display_classification(gp, "Classement final", 0);

    // Si tu veux sauvegarder ce classement dans un fichier, tu peux le faire ici
    // ex: FILE *f = fopen("results_sess_X.txt", "w"); ...
}

// Avance la session (P1->P2->P3->Q1->Q2->Q3->SPRINT->RACE->FINISHED)
void next_session(GPState *gp)
{
    switch(gp->currentSession){
        case SESS_NONE:
            gp->currentSession=SESS_P1;
            break;
        case SESS_P1:
            gp->currentSession=SESS_P2;
            break;
        case SESS_P2:
            gp->currentSession=SESS_P3;
            break;
        case SESS_P3:
            gp->currentSession=SESS_Q1;
            break;
        case SESS_Q1:
            gp->currentSession=SESS_Q2;
            break;
        case SESS_Q2:
            gp->currentSession=SESS_Q3;
            break;
        case SESS_Q3:
            if(gp->isSprint) gp->currentSession = SESS_SPRINT;
            else gp->currentSession = SESS_RACE;
            break;
        case SESS_SPRINT:
            gp->currentSession=SESS_RACE;
            break;
        case SESS_RACE:
        default:
            gp->currentSession=SESS_FINISHED;
            break;
    }
}

// Quand la session se termine, on donne les points selon le type de session
void end_session(GPState *gp)
{
    SessionType st = gp->currentSession;
    if(st==SESS_SPRINT){
        // top 8 : 8..1
        int sprintPts[8] = {8,7,6,5,4,3,2,1};
        for(int i=0; i<8; i++){
            gp->cars[i].points += sprintPts[i];
        }
    }
    else if(st==SESS_RACE){
        // top10 : 25..1
        int racePts[10] = {25,20,15,10,8,6,5,3,2,1};
        for(int i=0; i<10; i++){
            gp->cars[i].points += racePts[i];
        }
        // +1 point meilleur tour => la voiture #0 du classement
        gp->cars[0].points +=1;
    }
}

// Tri par bestLap
void sort_cars_by_bestlap(GPState *gp)
{
    for(int i=0; i<MAX_CARS-1; i++){
        for(int j=i+1; j<MAX_CARS; j++){
            if(gp->cars[j].bestLap < gp->cars[i].bestLap){
                Car tmp=gp->cars[i];
                gp->cars[i] = gp->cars[j];
                gp->cars[j] = tmp;
            }
        }
    }
}

// Affiche un tableau ASCII
// liveMode=1 => on peut afficher "LIVE" ou "provisoire"
// liveMode=0 => affichage final
void display_classification(const GPState *gp, const char *title, int liveMode)
{
    if(liveMode){
        printf("(LIVE) ");
    }
    print_ascii_table_header(title);
    for(int i=0; i<MAX_CARS; i++){
        print_ascii_table_row(i+1,
                              gp->cars[i].carNumber,
                              gp->cars[i].bestLap,
                              gp->cars[i].points);
    }
    print_ascii_table_footer();
}

// Sauvegarde finale
void save_final_classification(const GPState *gp, const char *filename)
{
    // Tri par points desc
    Car sorted[MAX_CARS];
    for(int i=0; i<MAX_CARS; i++){
        sorted[i] = gp->cars[i];
    }
    for(int i=0; i<MAX_CARS-1; i++){
        for(int j=i+1; j<MAX_CARS; j++){
            if(sorted[j].points>sorted[i].points){
                Car tmp=sorted[i];
                sorted[i]=sorted[j];
                sorted[j]=tmp;
            }
        }
    }

    FILE *f = fopen(filename, "w");
    if(!f){
        perror("fopen");
        return;
    }
    fprintf(f, "=== Classement final GP: %s ===\n", gp->gpName);
    fprintf(f, "POS | Car# | Points | BestLap\n");
    for(int i=0; i<MAX_CARS; i++){
        fprintf(f, "%2d  | %3d |   %3d  | %.3f\n",
                i+1,
                sorted[i].carNumber,
                sorted[i].points,
                sorted[i].bestLap);
    }
    fclose(f);
    printf("Classement final dans '%s'\n", filename);
}
