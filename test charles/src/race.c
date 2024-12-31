#include "race.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>

// Table des numéros de voitures
static int carNumbers[MAX_CARS] = {
    1,11,44,63,16,55,4,81,14,18,
    10,31,23,2,22,3,77,24,20,27
};

// Cherche si gpName (par ex. "Bahrain") matche un circuit
// Sinon prend circuits[0].
void init_new_gp(GPState *gp,
                 const char *gpName,
                 int isSprint,
                 Circuit circuits[],
                 int nbCirc)
{
    memset(gp, 0, sizeof(GPState));
    gp->isSprint = isSprint;
    gp->currentSession = SESS_P1;
    strncpy(gp->gpName, gpName, sizeof(gp->gpName));
    gp->gpName[sizeof(gp->gpName)-1] = '\0';

    // Rechercher un circuit dont le nom contient gpName
    int found = -1;
    for(int i=0; i<nbCirc; i++){
        if(strstr(circuits[i].nom, gpName)!=NULL) {
            found = i;
            break;
        }
    }
    if(found<0) {
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

    // On crée un pipe par voiture : pipefd[i][0]=lecture parent, [i][1]=écriture enfant
    int pipefd[MAX_CARS][2];
    for(int i=0; i<MAX_CARS; i++){
        if(pipe(pipefd[i])<0) {
            error_exit("pipe");
        }
    }

    // Fork 20 voitures
    for(int i=0; i<MAX_CARS; i++){
        pid_t p = fork();
        if(p<0){
            error_exit("fork");
        }
        else if(p==0){
            // Enfant : on ferme le "pipefd[i][0]" (lecture) et on garde pipefd[i][1] (écriture).
            close(pipefd[i][0]);
            // On lance le process
            run_car_process(i, pipefd[i][1]);
            // Jamais revenir ici
        } else {
            // Parent
            gp->cars[i].pid = p;
            // On ferme le "pipefd[i][1]" (écriture enfant) et on garde [i][0] (lecture).
            close(pipefd[i][1]);
        }
    }

    // On fait un rafraîchissement "live" toutes les 2s, pendant X secondes
    int duration=8;
    switch(gp->currentSession){
        case SESS_Q1: case SESS_Q2: case SESS_Q3:
            duration=6; break;
        case SESS_SPRINT:
            duration=5; break;
        case SESS_RACE:
            duration=10; break;
        default:
            duration=8; break;
    }

    // Pendant la session, on lit régulièrement les pipes
    // On va faire "duration" * 1000 ms / 2000 ms => steps
    int steps = duration*(1000/2000);

    for(int s=0; s<steps; s++){
        // On lit NON-BLOQUANT : si y a un lapTime, on met à jour bestLap
        for(int i=0; i<MAX_CARS; i++){
            double lapTime;
            // read(...) en mode "si pas de donnée, continue"
            int r = read(pipefd[i][0], &lapTime, sizeof(double));
            while(r>0) {
                // On a lu un lapTime
                if(lapTime<gp->cars[i].bestLap) {
                    gp->cars[i].bestLap = lapTime;
                }
                // On tente de relire s'il y a d'autres paquets
                r = read(pipefd[i][0], &lapTime, sizeof(double));
            }
            // s'il n'y a rien, on continue
        }

        // On trie pour un "live" scoreboard
        sort_cars_by_bestlap(gp);
        display_classification(gp, "Live scoreboard", 1);

        sleep(2);
    }

    // Fin de la session => kill les enfants
    for(int i=0; i<MAX_CARS; i++){
        if(gp->cars[i].pid>0){
            kill(gp->cars[i].pid, SIGTERM);
        }
    }
    // On wait
    for(int i=0; i<MAX_CARS; i++){
        if(gp->cars[i].pid>0){
            waitpid(gp->cars[i].pid, NULL, 0);
            gp->cars[i].pid = 0;
        }
    }

    // Lecture finale (au cas où y aurait des laps en buffer) => bestLap à jour
    for(int i=0; i<MAX_CARS; i++){
        double lapTime;
        while(read(pipefd[i][0], &lapTime, sizeof(double))>0) {
            if(lapTime<gp->cars[i].bestLap) {
                gp->cars[i].bestLap = lapTime;
            }
        }
        close(pipefd[i][0]);
    }

    // Tri final
    sort_cars_by_bestlap(gp);
    display_classification(gp, "Classement final de la session", 0);
}

// Avance la session
void next_session(GPState *gp)
{
    switch(gp->currentSession){
        case SESS_NONE:
            gp->currentSession=SESS_P1; break;
        case SESS_P1:
            gp->currentSession=SESS_P2; break;
        case SESS_P2:
            gp->currentSession=SESS_P3; break;
        case SESS_P3:
            gp->currentSession=SESS_Q1; break;
        case SESS_Q1:
            gp->currentSession=SESS_Q2; break;
        case SESS_Q2:
            gp->currentSession=SESS_Q3; break;
        case SESS_Q3:
            if(gp->isSprint) gp->currentSession=SESS_SPRINT;
            else gp->currentSession=SESS_RACE;
            break;
        case SESS_SPRINT:
            gp->currentSession=SESS_RACE; break;
        case SESS_RACE:
        default:
            gp->currentSession=SESS_FINISHED; break;
    }
}

// Attribue les points si c'est la sprint ou la course
void end_session(GPState *gp)
{
    SessionType st = gp->currentSession;
    // Sprint
    if(st==SESS_SPRINT){
        // top 8 => 8..1
        int sprintPts[8] = {8,7,6,5,4,3,2,1};
        for(int i=0; i<8; i++){
            gp->cars[i].points += sprintPts[i];
        }
    }
    else if(st==SESS_RACE){
        // top10 => 25..1
        int racePts[10] = {25,20,15,10,8,6,5,3,2,1};
        for(int i=0; i<10; i++){
            gp->cars[i].points += racePts[i];
        }
        // +1 point meilleur tour (on suppose la #1 du classement => bestLap plus rapide)
        gp->cars[0].points +=1;
    }
}

// Tri par bestLap
void sort_cars_by_bestlap(GPState *gp)
{
    for(int i=0; i<MAX_CARS-1; i++){
        for(int j=i+1; j<MAX_CARS; j++){
            if(gp->cars[j].bestLap < gp->cars[i].bestLap){
                Car tmp = gp->cars[i];
                gp->cars[i] = gp->cars[j];
                gp->cars[j] = tmp;
            }
        }
    }
}

// Affichage “beau tableau”
void display_classification(const GPState *gp, const char *title, int liveMode)
{
    if(liveMode){
        printf("\n-- %s (live) --\n", title);
    } else {
        printf("\n-- %s --\n", title);
    }
    print_ascii_table_header("");
    for(int i=0; i<MAX_CARS; i++){
        print_ascii_table_row(i+1,
                              gp->cars[i].carNumber,
                              gp->cars[i].bestLap,
                              gp->cars[i].points);
    }
    print_ascii_table_footer();
}

void save_final_classification(const GPState *gp, const char *filename)
{
    // Tri par points desc
    Car sorted[MAX_CARS];
    for(int i=0; i<MAX_CARS; i++){
        sorted[i] = gp->cars[i];
    }
    // tri desc
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
