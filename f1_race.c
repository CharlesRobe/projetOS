#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <math.h>

/* ---------------------------------------------------------------------------
 * Constantes et macros
 * --------------------------------------------------------------------------- */
#define MAX_CARS         20    /* Nombre de voitures qualifiées */
#define SECTORS_PER_LAP  3     /* Nombre de secteurs par tour */
#define PITSTOP_TIME_MIN 20    /* Ajout minimal lors d'un passage aux stands */
#define PITSTOP_TIME_MAX 30    /* Ajout maximal lors d'un passage aux stands */

#define SEM_MUTEX 0            /* Index du sémaphore pour un accès mutualisé */


#define ETAT_FILENAME "etat"
#define TAILLE_FICHIER "taillecircuit.txt"
#define QUALIFIED_FILE "qualifies.txt"

/* ---------------------------------------------------------------------------
 * Structures pour la mémoire partagée
 * --------------------------------------------------------------------------- */
const char* get_circuit_name_from_etat() {
    static char circuit_name[128];
    FILE *file = fopen(ETAT_FILENAME, "r");
    if (!file) {
        perror("Erreur lors de l'ouverture de 'etat'");
        exit(EXIT_FAILURE);
    }

    if (!fgets(circuit_name, sizeof(circuit_name), file)) {
        fprintf(stderr, "Erreur : Le fichier 'etat' est vide ou mal formé.\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // Supprime le retour à la ligne à la fin du nom, s'il existe
    circuit_name[strcspn(circuit_name, "\n")] = '\0';
    fclose(file);
    return circuit_name;
}

/* Informations sur une voiture */
typedef struct {
    int car_id;       
    int current_lap;   
    int current_sector; /* Secteur en cours (1, 2, ou 3) */
    double total_time;  /* Temps total depuis le début de la course */
    int status;         /* 0 = en course, 1 = aux stands, 2 = abandonnée */
    double timeS1;
    double timeS2;
    double timeS3;

    /* Historique des temps : [lap][sector], en secondes */
    double **history;   
} CarData;

/* Mémoire partagée globale */
typedef struct {
    CarData cars[MAX_CARS];       /* Données de toutes les voitures */
    double **leader_times;        /* Temps de passage du leader [lap][sector] */
    int last_displayed_lap;       /* Dernier tour affiché */
    int race_finished;            /* Indicateur de fin de course */
    
    double bestlap;
    int bestlapcar;

    int total_laps;               /* Nombre total de tours (env. 300 km) */
    double circuit_length;        /* Longueur du circuit (en km) */

    unsigned int random_seed;     /* Seed globale, utilisée et incrémentée par chaque fils */
} SharedMemory;

/* Union pour semctl (configuration des sémaphores) */
union semun {
    int              val;    
    struct semid_ds *buf;    
    unsigned short  *array;  
};

/* ---------------------------------------------------------------------------
 * Prototypes de fonctions
 * --------------------------------------------------------------------------- */
void read_initial_data(SharedMemory *shm,const char *circuit_name,int special);
void allocate_dynamic_arrays(SharedMemory *shm);
void free_dynamic_arrays(SharedMemory *shm);
void write_results_to_file(const char *circuit_name, SharedMemory *shm);
int  random_in_range(int min_val, int max_val);
void init_semaphore(int semid, int semnum, int init_val);
void semaphore_wait(int semid, int semnum);
void semaphore_signal(int semid, int semnum);
void car_process(int index, SharedMemory *shm, int semid);

static void display_tour_table(SharedMemory *shm, int current_lap);  /* Pour l'affichage en forme de tableau */

/* ---------------------------------------------------------------------------
 * Fonction principale
 * --------------------------------------------------------------------------- */
int f1_race_run(const char *circuit_name,int special)
{
    /* 1) Création de la clé pour la mémoire partagée et le(s) sémaphore(s) */
    key_t key = ftok(".", 'R');
    if (key == -1) {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    /* 2) Création de la mémoire partagée */
    int shmid = shmget(key, sizeof(SharedMemory), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    /* 3) Attachement au segment de mémoire partagée */
    SharedMemory *shm_ptr = (SharedMemory *) shmat(shmid, NULL, 0);
    if (shm_ptr == (void *) -1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    /* 4) Initialisation de la structure partagée */
    memset(shm_ptr, 0, sizeof(SharedMemory));
    shm_ptr->last_displayed_lap = -1; 
    shm_ptr->race_finished      = 0;  

    /* Lecture des données (longueur circuit, nb tours, voitures qualifiées...) */
    read_initial_data(shm_ptr,circuit_name,special);

    /* Allocation dynamique pour l’historique des temps et leader_times */
    allocate_dynamic_arrays(shm_ptr);

    /* On initialise la seed partagée : 
       Tous les fils vont l'incrémenter et s'en servir pour un srand(local_seed). */
    shm_ptr->random_seed = (unsigned int) time(NULL);

    /* Initialisation du CarData : on suppose qu’on a lu EXACTEMENT MAX_CARS = 10 qualifiés */
    for (int i = 0; i < MAX_CARS; i++) {
        shm_ptr->cars[i].current_lap    = 0;
        shm_ptr->cars[i].current_sector = 0;
        shm_ptr->cars[i].total_time     = 0.0;
        shm_ptr->cars[i].status         = 0;  /* 0=en course */

        /* Historique : allocation [total_laps][SECTORS_PER_LAP] */
        shm_ptr->cars[i].history = (double**) malloc(sizeof(double*) * shm_ptr->total_laps);
        for (int lap = 0; lap < shm_ptr->total_laps; lap++) {
            shm_ptr->cars[i].history[lap] = (double*) calloc(SECTORS_PER_LAP, sizeof(double));
        }
    }

    /* 5) Création + initialisation du sémaphore */
    int semid = semget(key, 1, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("semget");
        exit(EXIT_FAILURE);
    }
    init_semaphore(semid, SEM_MUTEX, 1);

    /* 6) Fork de processus fils (un par voiture) */
    pid_t pids[MAX_CARS];
    for (int i = 0; i < MAX_CARS; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        if (pid == 0) {
            /* --- PROCESSUS FILS --- */
            car_process(i, shm_ptr, semid);

            /* Libération mémoire (côté fils) */
            for (int lap = 0; lap < shm_ptr->total_laps; lap++) {
                free(shm_ptr->cars[i].history[lap]);
            }
            free(shm_ptr->cars[i].history);

            /* Détachement */
            shmdt(shm_ptr);
            exit(EXIT_SUCCESS);
        } else {
            /* --- PROCESSUS PERE --- */
            pids[i] = pid;
        }
    }

    /* 7) Le père attend la fin de tous les fils */
    for (int i = 0; i < MAX_CARS; i++) {
        waitpid(pids[i], NULL, 0);
    }

    /* On signale la fin de la course (si nécessaire) */
    semaphore_wait(semid, SEM_MUTEX);
    shm_ptr->race_finished = 1;
    semaphore_signal(semid, SEM_MUTEX);

    /* 8) Affichage et sauvegarde des résultats finaux */
    printf("\n----- Course terminée -----\n\n");
    /* Exemple d'affichage simple du résultat final */
    for (int i = 0; i < MAX_CARS; i++) {
        CarData *car = &shm_ptr->cars[i];
        printf("Voiture %2d | Temps total: %6.2f s | Statut: %s\n",
               car->car_id,
               car->total_time,
               (car->status == 2) ? "Abandonnée" :
               (car->status == 1) ? "Aux stands"  : "Terminé/En course");
    }

    /* Sauvegarde des résultats dans un fichier (simplifié) */
    write_results_to_file(circuit_name,shm_ptr);

    /* 9) Nettoyage de la mémoire partagée et des sémaphores */
    if (shmdt(shm_ptr) == -1) {
        perror("shmdt");
    }
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl");
    }
    if (semctl(semid, 0, IPC_RMID) == -1) {
        perror("semctl");
    }

    return 0;
}

/* ---------------------------------------------------------------------------
 * Fonction car_process : simulation d'une voiture (processus fils)
 * --------------------------------------------------------------------------- */
void car_process(int index, SharedMemory *shm, int semid)
{
    CarData *car = &shm->cars[index];
    int total_laps = shm->total_laps;

    /* Récupération et incrémentation de la seed pour ce fils */
    semaphore_wait(semid, SEM_MUTEX);
    unsigned int local_seed = shm->random_seed;
    shm->random_seed++;
    semaphore_signal(semid, SEM_MUTEX);

    /* Initialisation du générateur aléatoire de CE processus */
    srand(local_seed);

    while (car->current_lap < total_laps) {
        /* Parcours des 3 secteurs du tour */
        double lap=0;
        for (int s = 0; s < SECTORS_PER_LAP; s++) {
            /* Abandon aléatoire (1/1000) */
            if ((rand() % 1000) == 0) {
                semaphore_wait(semid, SEM_MUTEX);
                car->status = 2;  /* Abandonnée */
                semaphore_signal(semid, SEM_MUTEX);
                return;
            }
            

            /* Temps aléatoire du secteur [25..45] secondes */
            double sector_time = random_in_range(25000, 45000);
            
            sector_time=sector_time  / 1000;
            
          
            
            
            

            /* Dans le secteur 3 : 1 chance sur 5 d'aller aux stands */
            if (s == 2 && (rand() % 15) == 0) {
                semaphore_wait(semid, SEM_MUTEX);
                car->status = 1;   /* Aux stands */
                semaphore_signal(semid, SEM_MUTEX);

                int pitstop = random_in_range(PITSTOP_TIME_MIN, PITSTOP_TIME_MAX);
                sector_time += pitstop;  /* On ajoute le temps de stand */

                /* Sortie des stands */
                semaphore_wait(semid, SEM_MUTEX);
                car->status = 0;   /* Retour en course */
                semaphore_signal(semid, SEM_MUTEX);
            }
            semaphore_wait(semid, SEM_MUTEX);
            if (s==0){  
                car->timeS1=sector_time;
                }
            if (s==1){
                car->timeS2=sector_time;
                }
            if (s==2){
                car->timeS3=sector_time;
                }
            semaphore_signal(semid, SEM_MUTEX);
            /* Mise à jour de la mémoire partagée (section critique) */
            semaphore_wait(semid, SEM_MUTEX);

            car->current_sector = s + 1;
            car->history[car->current_lap][s] = (double) sector_time;
            car->total_time += (double) sector_time;
            
            lap += sector_time;

            /* Mise à jour du leader_times (si index=0 = "leader" forcé) */
            if (index == 0) {
                shm->leader_times[car->current_lap][s] = car->total_time;
            }

            semaphore_signal(semid, SEM_MUTEX);

            /* Sleep artificiel (optionnel) pour simuler un délai */
            usleep(100000);  /* 0.1s */
        }
        semaphore_wait(semid, SEM_MUTEX);

        // Comparaison et mise à jour du meilleur temps de tour
        if (shm->bestlap == 0 || lap < shm->bestlap) {
            shm->bestlap = lap;
            shm->bestlapcar = car->car_id;
        }

        // Libération du sémaphore après la section critique
        semaphore_signal(semid, SEM_MUTEX);
        /* La voiture a terminé un tour */
        semaphore_wait(semid, SEM_MUTEX);
  
        int current_lap = car->current_lap;
        /* Affichage global du tour si personne ne l'a déjà fait */
        if (shm->last_displayed_lap < current_lap) {
            display_tour_table(shm, current_lap);
            shm->last_displayed_lap = current_lap;
        }

        car->current_lap++;
        semaphore_signal(semid, SEM_MUTEX);
    }
}
// Fonction de comparaison (tri croissant par total_time)
int compare_cars(const void *a, const void *b) {
    CarData *carA = *(CarData **)a;
    CarData *carB = *(CarData **)b;
    if (carA->status != 2 && carB->status == 2)
        return -1; // carA avant carB
    if (carA->status == 2 && carB->status != 2)
        return 1;  // carB avant car
    if (carA->total_time < carB->total_time) 
        return -1;
    else if (carA->total_time > carB->total_time)
        return 1;
    else
        return 0;
}

// Fonction pour convertir le temps en minutes, secondes et millisecondes
void convert_time(double total_seconds, int *minutes, int *seconds, int *milliseconds) {
    if (total_seconds < 0) {
        *minutes = *seconds = *milliseconds = 0;
        return;
    }

    *minutes = (int)(total_seconds / 60);
    *seconds = (int)total_seconds % 60;
    *milliseconds = (int)((total_seconds - (*minutes * 60) - *seconds) * 1000);
}

/* ---------------------------------------------------------------------------
 * Affichage d’un tour sous forme de tableau
 * --------------------------------------------------------------------------- */
static void display_tour_table(SharedMemory *shm, int current_lap)
{
    // Créer un tableau de pointeurs vers toutes les voitures avec un temps valide
    CarData *car_ptrs[MAX_CARS];
    int actual_cars = 0;

    for (int c = 0; c < MAX_CARS; c++) {
        if (shm->cars[c].total_time > 0.0) { // Inclure toutes les voitures avec un temps valide
            car_ptrs[actual_cars++] = &shm->cars[c];
        }
    }

    // Trier le tableau de pointeurs en fonction du total_time
    qsort(car_ptrs, actual_cars, sizeof(CarData *), compare_cars);

    // Identifier le leader (voiture avec le plus petit total_time)
    CarData *leader = (actual_cars > 0) ? car_ptrs[0] : NULL;

    // Afficher la table
    printf("+ -----------------------------------------------------------------------------------------------------------+\n");
    printf("|                                         Affichage du Tour %-2d                                              |\n", current_lap + 1);
    printf("+--------+----------------+-----------------+-----------+------------+-----------+---------+----------------+\n");
    printf("|  ID    |  Time (m:s:ms) |  Écart (m:s:ms) |     S1    |      S2    |     S3    | Secteur |    Statut      |\n");
    printf("+--------+----------------+-----------------+-----------+------------+-----------+---------+----------------+\n");

    for (int c = 0; c < actual_cars; c++) {
        CarData *tmpCar = car_ptrs[c];
        double gap = 0.0;

        if (leader != NULL && tmpCar != leader) {  
            gap = tmpCar->total_time - leader->total_time;
        }

        // Détermination du statut en texte
        const char *statut_txt = (tmpCar->status == 2) ? "Abandonnée" :
                                 (tmpCar->timeS3 > 45) ? "PIT"  :
                                                         "En course";
        
        // Conversion du temps total
        int mintime, sectime, miletime;
        convert_time(tmpCar->total_time, &mintime, &sectime, &miletime);

        // Conversion de l'écart
        int gapmintime = 0, gapsectime = 0, gapmiletime = 0;
        if (tmpCar != leader) {
            convert_time(gap, &gapmintime, &gapsectime, &gapmiletime);
        }

        // Affichage du temps formaté
        printf("| %6d | %2d:%02d:%03d      | ", 
               tmpCar->car_id, mintime, sectime, miletime);

        if (tmpCar == leader) {
            printf("  0:00:000      |");
        } else {
            printf(" %2d:%02d:%03d      |", gapmintime, gapsectime, gapmiletime);
        }
        printf("   %.3f  |   %.3f   |   %.3f  |",tmpCar->timeS1,tmpCar->timeS2,tmpCar->timeS3);
        printf("   %2d    | %-14s |\n",
               tmpCar->current_sector,
               statut_txt);
    }

    printf("+--------+----------------+-----------------+-----------+------------+-----------+---------+----------------+\n");
    printf("Le meilleur tour est de %.3f par la voiture %d \n",shm->bestlap,shm->bestlapcar);
}

/* ---------------------------------------------------------------------------
 * Lecture des données (circuit, voitures qualifiées) - Simplifiée
 * --------------------------------------------------------------------------- */
void read_initial_data(SharedMemory *shm, const char *circuit_name,int special) {
    // Construire le chemin vers taillecircuit.txt
    char taille_file_path[256];
    snprintf(taille_file_path, sizeof(taille_file_path),
             "%s/taillecircuit.txt", circuit_name);

    // Ouvrir taillecircuit.txt
    FILE *file = fopen(taille_file_path, "r");
    if (!file) {
        perror("Erreur lors de l'ouverture de taillecircuit.txt");
        exit(EXIT_FAILURE);
    }
    if (fscanf(file, "%lf", &shm->circuit_length) != 1) {
        fprintf(stderr, "Erreur : impossible de lire la taille du circuit.\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }
    fclose(file);

    // Calcul du nombre de tours pour ~300 km
    printf("circuit_length %f",shm->circuit_length);
    int km=100000;
    if (!special){km=300000;}
    
    shm->total_laps = (int)(km / shm->circuit_length + 0.5);

    printf("Longueur du circuit : %.3f km\n", shm->circuit_length);
    printf("Nombre de tours : %d\n", shm->total_laps);

    // Construire le chemin vers qualifies.txt
    char qualif_file_path[256];
    snprintf(qualif_file_path, sizeof(qualif_file_path),
             "%s/qualifies.txt", circuit_name);
    
    // Lire la liste des voitures qualifiées
    //file = fopen(qualif_file_path, "r");  TODO changer après quand les qualifs creent un fichier avec les qualifiés
    file = fopen("qualifies.txt","r");
    if (!file) {
        perror("Erreur lors de l'ouverture de qualifies.txt");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < MAX_CARS; i++) {
        if (fscanf(file, "%d", &shm->cars[i].car_id) != 1) {
            fprintf(stderr, "Erreur : impossible de lire la voiture %d.\n", i+1);
            fclose(file);
            exit(EXIT_FAILURE);
        }
        printf("Voiture qualifiée : %d\n", shm->cars[i].car_id);
    }
    fclose(file);
}

/* ---------------------------------------------------------------------------
 * Allocation dynamique pour leader_times, selon total_laps
 * --------------------------------------------------------------------------- */
void allocate_dynamic_arrays(SharedMemory *shm)
{
    int laps = shm->total_laps;
    shm->leader_times = (double**) malloc(sizeof(double*) * laps);
    for (int lap = 0; lap < laps; lap++) {
        shm->leader_times[lap] = (double*) calloc(SECTORS_PER_LAP, sizeof(double));
    }
}

/* ---------------------------------------------------------------------------
 * Libération (si besoin) des tableaux dynamiques
 * --------------------------------------------------------------------------- */
void free_dynamic_arrays(SharedMemory *shm)
{
    if (!shm->leader_times) return;
    for (int lap = 0; lap < shm->total_laps; lap++) {
        free(shm->leader_times[lap]);
    }
    free(shm->leader_times);
}

/* ---------------------------------------------------------------------------
 * Écriture des résultats finaux
 * --------------------------------------------------------------------------- */
void write_results_to_file(const char *circuit_name, SharedMemory *shm)
{
    char classement_path[256];
    snprintf(classement_path, sizeof(classement_path),
             "%s/classement.txt", circuit_name);

    FILE *f = fopen(classement_path, "w");
    if (!f) {
        perror("Erreur lors de la création de classement.txt");
        return;
    }

    fprintf(f, "--- Résultats de la Course pour le circuit '%s' ---\n", circuit_name);
    fprintf(f, "-----------------------------------------------\n");
    for (int i = 0; i < MAX_CARS; i++) {
        fprintf(f, "Voiture %d - Temps total: %.2f s - Statut: %d\n",
                shm->cars[i].car_id,
                shm->cars[i].total_time,
                shm->cars[i].status);
    }

    fclose(f);
    printf("Résultats écrits dans %s\n", classement_path);
}

/* ---------------------------------------------------------------------------
 * Génération d’un entier aléatoire dans [min_val, max_val]
 * --------------------------------------------------------------------------- */
int random_in_range(int min_val, int max_val)
{
    return min_val + rand() % (max_val - min_val + 1);
}

/* ---------------------------------------------------------------------------
 * Initialise un sémaphore à la valeur init_val
 * --------------------------------------------------------------------------- */
void init_semaphore(int semid, int semnum, int init_val)
{
    union semun arg;
    arg.val = init_val;
    if (semctl(semid, semnum, SETVAL, arg) == -1) {
        perror("semctl SETVAL");
        exit(EXIT_FAILURE);
    }
}

/* ---------------------------------------------------------------------------
 * Opération "wait" (P) sur un sémaphore
 * --------------------------------------------------------------------------- */
void semaphore_wait(int semid, int semnum)
{
    struct sembuf sops;
    sops.sem_num = semnum;
    sops.sem_op  = -1;  /* P */
    sops.sem_flg = 0;
    if (semop(semid, &sops, 1) == -1) {
        perror("semop wait");
        exit(EXIT_FAILURE);
    }
}

/* ---------------------------------------------------------------------------
 * Opération "signal" (V) sur un sémaphore
 * --------------------------------------------------------------------------- */
void semaphore_signal(int semid, int semnum)
{
    struct sembuf sops;
    sops.sem_num = semnum;
    sops.sem_op  = 1;   /* V */
    sops.sem_flg = 0;
    if (semop(semid, &sops, 1) == -1) {
        perror("semop signal");
        exit(EXIT_FAILURE);
    }
}

