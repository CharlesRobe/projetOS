#include "race.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <math.h>

static int carNumbers[MAX_CARS] = {
    1, 11, 44, 63, 16, 55, 4, 81, 14, 18,
    10, 31, 23, 2, 22, 3, 77, 24, 20, 27
};

// Rechercher dans circuits[] un circuit dont le nom matche gpName
// Sinon prendre circuits[0]
void init_new_gp(GrandPrixWeekend *gpw,
                 const char *gpName,
                 int isSprint,
                 Circuit circuits[],
                 int nbC)
{
    memset(gpw, 0, sizeof(GrandPrixWeekend));
    gpw->isSprint = isSprint;
    gpw->currentSession = SESS_P1;
    strncpy(gpw->gpName, gpName, sizeof(gpw->gpName));
    gpw->gpName[sizeof(gpw->gpName)-1] = '\0';

    // Chercher correspondance
    int found = -1;
    for(int i=0; i<nbC; i++) {
        if(strstr(circuits[i].nom, gpName) != NULL) {
            found = i;
            break;
        }
    }
    if(found < 0) {
        // Pas trouvé => on prend le 1er circuit
        gpw->circuit = circuits[0];
    } else {
        gpw->circuit = circuits[found];
    }

    // Init voitures
    for(int i=0; i<MAX_CARS; i++){
        gpw->cars[i].pid = 0;
        gpw->cars[i].carNumber = carNumbers[i];
        gpw->cars[i].status = CAR_RUNNING;
        gpw->cars[i].bestLap = 9999.0;
        gpw->cars[i].points = 0;
    }
}

// Lance la session courante
void run_session(GrandPrixWeekend *gpw)
{
    if(gpw->currentSession == SESS_NONE || gpw->currentSession == SESS_FINISHED) {
        printf("Aucune session à lancer.\n");
        return;
    }

    printf("\n=== [GP: %s | Session: %d] ===\n", gpw->gpName, gpw->currentSession);

    // Forker 20 voitures
    for(int i=0; i<MAX_CARS; i++){
        pid_t p = fork();
        if(p<0){
            error_exit("fork");
        } else if(p==0){
            // Enfant
            run_car_process(gpw->cars[i].carNumber);
        } else {
            gpw->cars[i].pid = p;
        }
    }

    // Durée variable
    int duration = 8;
    switch(gpw->currentSession){
        case SESS_Q1: case SESS_Q2: case SESS_Q3:
            duration = 6; break;
        case SESS_SPRINT:
            duration = 5; break;
        case SESS_RACE:
            duration = 10; break;
        default:
            duration = 8; break;
    }

    // Refresh “live” toutes les 2s
    int steps = duration * (1000/2000);
    for(int s=0; s<steps; s++){
        printf("[Parent] Refresh session %d (t=%d)\n", gpw->currentSession, s);
        sleep(2);
    }

    // Fin => kill
    for(int i=0; i<MAX_CARS; i++){
        if(gpw->cars[i].pid>0){
            kill(gpw->cars[i].pid, SIGTERM);
        }
    }
    // Wait
    for(int i=0; i<MAX_CARS; i++){
        if(gpw->cars[i].pid>0){
            waitpid(gpw->cars[i].pid, NULL, 0);
            gpw->cars[i].pid=0;
        }
    }

    // Tri
    sort_cars_by_bestlap(gpw);

    // Affichage final
    char title[128];
    sprintf(title, "Classement final de la session %d", gpw->currentSession);
    display_classification(gpw, title);
}

// Passe à la prochaine session
void next_session(GrandPrixWeekend *gpw)
{
    switch(gpw->currentSession){
        case SESS_NONE:
            gpw->currentSession = SESS_P1;
            break;
        case SESS_P1:
            gpw->currentSession = SESS_P2;
            break;
        case SESS_P2:
            gpw->currentSession = SESS_P3;
            break;
        case SESS_P3:
            gpw->currentSession = SESS_Q1;
            break;
        case SESS_Q1:
            gpw->currentSession = SESS_Q2;
            break;
        case SESS_Q2:
            gpw->currentSession = SESS_Q3;
            break;
        case SESS_Q3:
            if(gpw->isSprint) gpw->currentSession = SESS_SPRINT;
            else gpw->currentSession = SESS_RACE;
            break;
        case SESS_SPRINT:
            gpw->currentSession = SESS_RACE;
            break;
        case SESS_RACE:
        default:
            gpw->currentSession = SESS_FINISHED;
            break;
    }
}

// Attribue les points si besoin
void end_session(GrandPrixWeekend *gpw)
{
    SessionType st = gpw->currentSession;
    if(st==SESS_SPRINT){
        // top8 => 8..1
        int sprintPts[8] = {8,7,6,5,4,3,2,1};
        for(int i=0; i<8; i++){
            gpw->cars[i].points += sprintPts[i];
        }
    } else if(st==SESS_RACE){
        // top10 => 25..1
        int racePts[10] = {25,20,15,10,8,6,5,3,2,1};
        for(int i=0; i<10; i++){
            gpw->cars[i].points += racePts[i];
        }
        // +1 point meilleur tour (on simule le #1 a le bestlap minimal)
        gpw->cars[0].points += 1; 
    }
}

// Tri local par bestLap
void sort_cars_by_bestlap(GrandPrixWeekend *gpw)
{
    for(int i=0; i<MAX_CARS-1; i++){
        for(int j=i+1; j<MAX_CARS; j++){
            if(gpw->cars[j].bestLap < gpw->cars[i].bestLap){
                Car tmp = gpw->cars[i];
                gpw->cars[i] = gpw->cars[j];
                gpw->cars[j] = tmp;
            }
        }
    }
}

// Affichage d’un beau tableau ASCII
void display_classification(const GrandPrixWeekend *gpw, const char *title)
{
    print_ascii_table_header(title);
    for(int i=0; i<MAX_CARS; i++){
        print_ascii_table_row(i+1,
                              gpw->cars[i].carNumber,
                              gpw->cars[i].bestLap,
                              gpw->cars[i].points);
    }
    print_ascii_table_footer();
}

// Sauvegarde finale
void save_final_classification(const GrandPrixWeekend *gpw, const char *filename)
{
    // On trie par points (desc)
    Car sorted[MAX_CARS];
    for(int i=0; i<MAX_CARS; i++){
        sorted[i] = gpw->cars[i];
    }
    // tri desc points
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
    fprintf(f, "=== Classement final GP: %s ===\n", gpw->gpName);
    fprintf(f, "POS | Car# | Points | BestLap\n");
    for(int i=0; i<MAX_CARS; i++){
        fprintf(f, "%2d  | %3d |  %3d   | %.3f\n",
                i+1,
                sorted[i].carNumber,
                sorted[i].points,
                sorted[i].bestLap);
    }
    fclose(f);
}
