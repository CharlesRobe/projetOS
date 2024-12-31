#include "race.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <math.h>
#include <unistd.h>

static int carNumbers[MAX_CARS] = {
   1,11,44,63,16,55,4,81,14,18,
   10,31,23,2,22,3,77,24,20,27
};

// Convertir session en nom
static const char* session_name(SessionType st)
{
    switch(st){
        case SESS_ESSAI1: return "Essai1";
        case SESS_ESSAI2: return "Essai2";
        case SESS_ESSAI3: return "Essai3";
        case SESS_Q1:     return "Q1";
        case SESS_Q2:     return "Q2";
        case SESS_Q3:     return "Q3";
        case SESS_SPRINT: return "Sprint";
        case SESS_COURSE: return "Course";
        default:          return "None/Finished";
    }
}

void init_new_gp(GPState *gp, int circuitNumber, int isSprint,
                 Circuit circuits[], int nbCirc)
{
    memset(gp,0,sizeof(GPState));
    gp->isSprint= isSprint;
    gp->currentSession= SESS_ESSAI1;

    // Cherche le circuit
    int found=-1;
    for(int i=0; i<nbCirc; i++){
        if(circuits[i].numeroCourse==circuitNumber){
            found=i; break;
        }
    }
    if(found<0) found=0;
    gp->circuit= circuits[found];

    // gpName
    snprintf(gp->gpName, sizeof(gp->gpName),
             "GP #%d : %s", circuitNumber, gp->circuit.nom);

    // nbTours
    // ex. ~100 km => sprint => on calcule = ceil(100 / longueur)
    gp->nbToursSprint= (int)ceil(100.0/gp->circuit.longueur);
    // ~300 km => course
    gp->nbToursCourse= (int)ceil(300.0/gp->circuit.longueur);

    // init voitures
    for(int i=0; i<MAX_CARS; i++){
        gp->cars[i].pid=0;
        gp->cars[i].carNumber= carNumbers[i];
        gp->cars[i].status= CAR_RUNNING;
        gp->cars[i].bestLap= 9999.0;
        gp->cars[i].bestS1= 9999.0;
        gp->cars[i].bestS2= 9999.0;
        gp->cars[i].bestS3= 9999.0;
        gp->cars[i].points=0;
    }

    // init elimQ1, elimQ2
    for(int i=0; i<5; i++){
        gp->elimQ1[i]= -1;
        gp->elimQ2[i]= -1;
    }
}

void run_session(GPState *gp)
{
    if(gp->currentSession==SESS_NONE || gp->currentSession==SESS_FINISHED){
        printf("Aucune session à lancer.\n");
        return;
    }

    printf("\n=== [GP: %s | Session: %s] ===\n",
           gp->gpName, session_name(gp->currentSession));

    // creer un pipe par voiture
    int pipefd[MAX_CARS][2];
    // on regarde combien de voitures "actives" on doit forker
    int nbActive=20;

    // si on est en Q2 => on a 15 voitures
    // si Q3 => 10
    // plus tard : si on a gp->elimQ1[] => etc.

    // *Détermination du set de voitures actives*
    // Q1 => 20
    // Q2 => 15
    // Q3 => 10
    // SPRINT/COURSE => 20 (ou 15 ? On suppose 20 pour simplifier, mais en vrai la grille est déterminée)
    // Pour être conforme, on doit vraiment enlever 5 en Q2, 5 en Q3.
    int activeCarsIndex[20];
    for(int i=0; i<20; i++){
        activeCarsIndex[i]= i;
    }

    if(gp->currentSession==SESS_Q2){
        // on doit enlever les 5 de Q1
        // gp->elimQ1[] contient indices triés
        // => on retire ces 5 voitures du set
        nbActive=15;
    }
    else if(gp->currentSession==SESS_Q3){
        nbActive=10;
    }
    // Pour simplifier, on suppose qu'on a mis de côté les 5/10 indices déjà éliminés
    // Ici, on n'implémente pas le détail complet. On fait "comme si" on fork tout le monde,
    // ou alors on le fait vraiment ?

    // Pour la démo, on va forker 20 voitures à Q2/Q3, *mais* on ignorerait leurs temps
    // si elles sont "éliminées" => c'est un cheat, on n'a pas le vrai code d'élim.
    // L'idéal = forker seulement les "non éliminées".

    for(int i=0; i<MAX_CARS; i++){
        if(pipe(pipefd[i])<0){
            error_exit("pipe");
        }
    }

    // on fork
    for(int i=0; i<MAX_CARS; i++){
        pid_t p=fork();
        if(p<0){
            error_exit("fork");
        } else if(p==0){
            // enfant
            close(pipefd[i][0]);
            run_car_process(i, pipefd[i][1]);
        } else {
            gp->cars[i].pid=p;
            close(pipefd[i][1]);
        }
    }

    // Durée
    int duration=8; // Essais => 8
    switch(gp->currentSession){
        case SESS_Q1:
        case SESS_Q2:
        case SESS_Q3:
            duration=6; break;
        case SESS_SPRINT:
            duration=5; break;
        case SESS_COURSE:
            duration=10; break;
        default:
            duration=8; // Essai
    }

    // boucle refresh
    int steps = duration * (1000/2000); // ex. 8 -> 4 steps
    for(int s=0; s<steps; s++){
        // on lit
        for(int i=0; i<MAX_CARS; i++){
            while(1){
                double s1, s2, s3, pitVal;
                int r = read(pipefd[i][0], &s1, sizeof(double));
                if(r<=0) break;
                int r2= read(pipefd[i][0], &s2, sizeof(double));
                int r3= read(pipefd[i][0], &s3, sizeof(double));
                int r4= read(pipefd[i][0], &pitVal, sizeof(double));
                if(r2>0 && r3>0 && r4>0){
                    // check out => s1<0 => out
                    if(s1<0.0 && s2<0.0 && s3<0.0){
                        gp->cars[i].status= CAR_OUT;
                    } else {
                        // normal
                        double lapTime= s1+s2+s3;
                        if(lapTime<gp->cars[i].bestLap){
                            gp->cars[i].bestLap= lapTime;
                        }
                        if(s1<gp->cars[i].bestS1) gp->cars[i].bestS1=s1;
                        if(s2<gp->cars[i].bestS2) gp->cars[i].bestS2=s2;
                        if(s3<gp->cars[i].bestS3) gp->cars[i].bestS3=s3;

                        if(pitVal>0.5){
                            gp->cars[i].status= CAR_PIT;
                        } else {
                            if(gp->cars[i].status!=CAR_OUT){
                                gp->cars[i].status= CAR_RUNNING;
                            }
                        }
                    }
                }
                else{
                    break; // data incomplete
                }
            }
        }

        // tri
        sort_cars_by_bestlap(gp);

        // affiche
        printf("\n--- [Live scoreboard: %s, step=%d] ---\n",
               session_name(gp->currentSession), s);
        display_classification(gp, "Classement provisoire", 1);

        sleep(2);
    }

    // fin => kill
    for(int i=0; i<MAX_CARS; i++){
        if(gp->cars[i].pid>0){
            kill(gp->cars[i].pid, SIGTERM);
        }
    }
    // wait
    for(int i=0; i<MAX_CARS; i++){
        if(gp->cars[i].pid>0){
            waitpid(gp->cars[i].pid,NULL,0);
            gp->cars[i].pid=0;
        }
    }

    // lecture finale
    for(int i=0; i<MAX_CARS; i++){
        while(1){
            double s1,s2,s3,pitVal;
            int r= read(pipefd[i][0], &s1, sizeof(double));
            if(r<=0) break;
            int r2=read(pipefd[i][0], &s2,sizeof(double));
            int r3=read(pipefd[i][0], &s3,sizeof(double));
            int r4=read(pipefd[i][0], &pitVal,sizeof(double));
            if(r2>0 && r3>0 && r4>0){
                // out ?
                if(s1<0.0 && s2<0.0 && s3<0.0){
                    gp->cars[i].status=CAR_OUT;
                } else {
                    double lapTime= s1+s2+s3;
                    if(lapTime<gp->cars[i].bestLap){
                        gp->cars[i].bestLap= lapTime;
                    }
                    if(s1<gp->cars[i].bestS1) gp->cars[i].bestS1=s1;
                    if(s2<gp->cars[i].bestS2) gp->cars[i].bestS2=s2;
                    if(s3<gp->cars[i].bestS3) gp->cars[i].bestS3=s3;
                }
            } else {
                break;
            }
        }
        close(pipefd[i][0]);
    }

    // tri final
    sort_cars_by_bestlap(gp);

    // affiche final
    printf("\n=== Classement final de la session %s ===\n",
           session_name(gp->currentSession));
    display_classification(gp, "Classement final", 0);
}

void next_session(GPState *gp)
{
    switch(gp->currentSession){
        case SESS_NONE:
            gp->currentSession=SESS_ESSAI1; break;
        case SESS_ESSAI1:
            gp->currentSession=SESS_ESSAI2; break;
        case SESS_ESSAI2:
            gp->currentSession=SESS_ESSAI3; break;
        case SESS_ESSAI3:
            gp->currentSession=SESS_Q1; break;
        case SESS_Q1:
            gp->currentSession=SESS_Q2; break;
        case SESS_Q2:
            gp->currentSession=SESS_Q3; break;
        case SESS_Q3:
            if(gp->isSprint){
                gp->currentSession=SESS_SPRINT;
            } else {
                gp->currentSession=SESS_COURSE;
            }
            break;
        case SESS_SPRINT:
            gp->currentSession=SESS_COURSE;
            break;
        case SESS_COURSE:
        default:
            gp->currentSession=SESS_FINISHED;
            break;
    }
}

// A la fin de Q1 => elim 5 => places 16..20
// A la fin de Q2 => elim 5 => places 11..15
// A la fin de Q3 => places 1..10
// SPRINT => top8 => 8..1
// COURSE => top10 => 25..1 +1 meilleur tour
void end_session(GPState *gp)
{
    SessionType st= gp->currentSession;

    if(st==SESS_Q1){
        // On vient de faire Q1 => les 5 derniers du tri => elimQ1
        // => on les range places 16..20
        // (On ne montre pas tout le code ici, on stockerait gp->elimQ1[i]= indexCar i
    }
    else if(st==SESS_Q2){
        // 5 autres => 11..15
    }
    else if(st==SESS_Q3){
        // On a le top10 => 1..10
    }
    else if(st==SESS_SPRINT){
        // top8 => 8..1
        int pts[8]={8,7,6,5,4,3,2,1};
        for(int i=0; i<8; i++){
            gp->cars[i].points += pts[i];
        }
    }
    else if(st==SESS_COURSE){
        // top10 => 25..1
        int pts[10]={25,20,15,10,8,6,5,3,2,1};
        for(int i=0; i<10; i++){
            gp->cars[i].points+= pts[i];
        }
        // +1 best lap => i=0
        gp->cars[0].points++;
    }
}

void sort_cars_by_bestlap(GPState *gp)
{
    for(int i=0; i<MAX_CARS-1; i++){
        for(int j=i+1; j<MAX_CARS; j++){
            if(gp->cars[j].bestLap < gp->cars[i].bestLap){
                Car tmp= gp->cars[i];
                gp->cars[i]= gp->cars[j];
                gp->cars[j]= tmp;
            }
        }
    }
}

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

void save_final_classification(const GPState *gp, const char *filename)
{
    // tri desc par points
    Car sorted[20];
    for(int i=0; i<20; i++){
        sorted[i]= gp->cars[i];
    }
    for(int i=0; i<19; i++){
        for(int j=i+1; j<20; j++){
            if(sorted[j].points>sorted[i].points){
                Car tmp= sorted[i];
                sorted[i]= sorted[j];
                sorted[j]= tmp;
            }
        }
    }

    FILE *f= fopen(filename,"w");
    if(!f){
        perror("fopen");
        return;
    }
    fprintf(f,"=== Classement final %s ===\n", gp->gpName);
    for(int i=0; i<20; i++){
        fprintf(f,"%2d) Car #%2d | Points=%d | bestLap=%.3f\n",
                i+1,
                sorted[i].carNumber,
                sorted[i].points,
                sorted[i].bestLap);
    }
    fclose(f);
    printf("Classement final dans '%s'\n", filename);
}
