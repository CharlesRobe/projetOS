#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>
#include <math.h>

float GenRanNum(int seedIncrementer, int secMin, int secMax) { //Génére un temps aléatoire compris entre secMin et secMan secondes.
	srand(time(NULL) + seedIncrementer);
	/*Se base sur le temps système pour générer une seed qui fournit une séquence de nombres aléatoires.
	seedIncrementer incrémente le srand pour éviter que la fonction aléatoire sorte systématiquement la même chose. */

	//printf("%lu", time(NULL) + seedIncrementer);

	float randomTime = ((rand() % (((secMax - secMin)*1000) + 1) ) + (secMin*1000)) / 1000.0; //Donner un temps compris entre 25 et 45 secondes pour un secteur.

	//printf("Temps généré par la fonction GenRanNum: %.3f\n", randomTime);

	return randomTime;
}




int doingLap(int *graine, float *tempsSec1, float *tempsSec2, float *tempsSec3, float *tempsTotal) { //Appelée par une voiture → Lance une simulation d'un tour, il peut aboutir en temps, en un crash ou en pit.
	float bestTime = INFINITY;
	int seed = *graine;
	int diceResult = GenRanNum(seed++,1,100);
	printf("Valeur générée par le dé servant à déterminer l'aboutissement du tour: %d.\n", diceResult);

	// Traitement de la chance que le pilote se plante.
	if ( diceResult == 100 ) { // Si le dé match le 1% de crash, on entame l'algorithme qui détermine à quel secteur l'incident est arrivé.
		int sectorResult = GenRanNum(seed++,1,100); // Pour déterminer dans quel secteur l'accident a eu lieu.
		printf("Valeur générée par le dé servant à déterminer la localisation du crash : %d.\n", sectorResult);
		*tempsTotal = INFINITY; // Car dans tous les cas le tour ne sera pas terminé.
        // int pitState = 0;
		if ( sectorResult <= 33 ) { // Si crash au secteur 1 → Aucun secteur ne reçoit de temps
			printf("L'accident a eu lieu dans le secteur 1.\n\n");
			*tempsSec1 = INFINITY;
			*tempsSec2 = INFINITY;
			*tempsSec3 = INFINITY;
		}
		else if ( sectorResult <= 66 ) { // Si crash au secteur 2 → Seul secteur 1 reçoit un temps.
			printf("L'accident a eu lieu dans le secteur 2.\n\n");
			*tempsSec1 = GenRanNum(seed++, 25, 45); // Secteur 1 reçoit un temps.
			*tempsSec2 = INFINITY;
			*tempsSec3 = INFINITY;
		}
		else { // Si crash au secteur 3 → Seul secteur 3 ne reçoit pas de temps.
			printf("L'accident a eu lieu dans le secteur 3.\n\n");
			*tempsSec1 = GenRanNum(seed++, 25, 45); // Secteur 1 reçoit un temps.
			*tempsSec2 = GenRanNum(seed++, 25, 45); // Secteur 2 aussi.
			*tempsSec3 = INFINITY;
		}
	}

	// Traitement de la chance que le pilote retourne aux stands (else if car s'il ne s'est pas crashé, il est peut-être retourné aux stands)
	else if ( diceResult > 84 ) { // Si le résultat du dé est supérieur à 84 → Le pilote veut retourner aux stands.
		printf("Le pilote a décidé de retourner aux stands.\n\n");
		*tempsSec1 = GenRanNum(seed++, 25, 45); // Secteur 1 reçoit un temps.
		*tempsSec2 = GenRanNum(seed++, 25, 45); // Secteur 2 aussi.
		*tempsSec3 = 3600; // Il faut générer un temps aux stands. 3600 - temps qui est déjà passé -135 secondes
	}
	else {
		printf("Tour normal.\n");
		*tempsSec1 = GenRanNum(seed++, 25, 45);
		*tempsSec2 = GenRanNum(seed++, 25, 45);
		*tempsSec3 = GenRanNum(seed++, 25, 45);

		// Conversion d'un total en secondes en un affichage min:sec:msec.
		*tempsTotal = *tempsSec1 + *tempsSec2 + *tempsSec3;
		int tempsMinutes = *tempsTotal / 60.0;
		int tempsSecondes = *tempsTotal - (tempsMinutes * 60);
		int tempsMsecondes = (*tempsTotal - (tempsMinutes * 60) - tempsSecondes) * 1000;
		
		printf("Temps généré: %d:%d:%d = %.3f secondes\n\n", tempsMinutes, tempsSecondes, tempsMsecondes, *tempsTotal);
	}
	*graine = seed;
	//int outputArray[] = {tempsSec1, tempsSec2, tempsSec3, tempsTotal};
	//return outputArray;
}


int Essais() {
	// Initialise les différentes voitures.
	// Truc qui sort les meilleur temps de voiture à la fin des essais.

	// Variables qui contiendront les meilleurs temps.
	float bestTimeSec1 = INFINITY;
	float bestTimeSec2 = INFINITY;
	float bestTimeSec3 = INFINITY;
	float bestTimeTot = INFINITY;
	
	// Variables qui seront mises à jour par doingLap() à chaque fois qu'une voiture effectue un tour.
	float tempsSec1, tempsSec2, tempsSec3, tempsTotal;
	int seed=0;
	
	
	int seed4GenRanTime = 0; // Utilisée comme graine par GenRanNum() et incrémentée à chaque utilisation.
	float trialsDuration = 0; // Variable qui compte la progression temporelle des essais.
	if (GenRanNum(seed4GenRanTime++, 0, 100) < 20) { // La voiture décide-t-elle de commencer plus tard que le commencement des essais ?
		trialsDuration = GenRanNum(seed4GenRanTime++, 0, 40); // De combien de temps ? REVENIR DESSUS
	}
	
	while (trialsDuration < 3600) {
		doingLap(&seed	, &tempsSec1, &tempsSec2, &tempsSec3, &tempsTotal);
		printf("Valeurs des meilleurs temps: %.3f, %.3f, %.3f, %.3f.\n", bestTimeSec1, bestTimeSec2, bestTimeSec3, bestTimeTot);
		printf("Temps effectués par la voiture: %.3f, %.3f, %.3f, %.3f. duree essais %.3f \n", tempsSec1, tempsSec2, tempsSec3, tempsTotal, trialsDuration);
		trialsDuration += tempsTotal;	
		if (tempsSec1 < bestTimeSec1){
			bestTimeSec1 = tempsSec1;}
		if (tempsSec2 < bestTimeSec2){
			bestTimeSec2 = tempsSec2;}
		if (tempsSec3 < bestTimeSec3){
			bestTimeSec3 = tempsSec3;}
		if (tempsTotal < bestTimeTot) {
			bestTimeTot=tempsTotal;}
	}
}


void qualifs(const char* sessionName, int durationMinutes, int seed) {
	// Fonction qui sort les meilleur temps de voiture à la fin des qualifs.
    printf("Début de la session %s.\n", sessionName);

	// Variables qui contiendront les meilleurs temps.
    float bestTimeSec1 = INFINITY;
    float bestTimeSec2 = INFINITY;
    float bestTimeSec3 = INFINITY;
    float bestTimeTot = INFINITY;

    // Variables qui seront mises à jour par doingLap() à chaque fois qu'une voiture effectue un tour.
    float tempsSec1, tempsSec2, tempsSec3, tempsTotal;

    // Initialisation de la variable qui nous sert à contrôler le temps de la session
    float sessionDuration = 0;

    // Simulation des tours et enregistrements des meilleuts temps en temps réel
    while (sessionDuration < durationMinutes * 60) {
        doingLap(&seed, &tempsSec1, &tempsSec2, &tempsSec3, &tempsTotal);
        printf("Valeurs des meilleurs temps: %.3f, %.3f, %.3f, %.3f.\n", bestTimeSec1, bestTimeSec2, bestTimeSec3, bestTimeTot);
        printf("Temps effectués par la voiture: %.3f, %.3f, %.3f, %.3f. Durée %s : %.3f \n", tempsSec1, tempsSec2, tempsSec3, tempsTotal,sessionName, sessionDuration);
        sessionDuration += tempsTotal;
        if (tempsSec1 < bestTimeSec1) {
            bestTimeSec1 = tempsSec1;
        }
        if (tempsSec2 < bestTimeSec2) {
            bestTimeSec2 = tempsSec2;
        }
        if (tempsSec3 < bestTimeSec3) {
            bestTimeSec3 = tempsSec3;
        }
        if (tempsTotal < bestTimeTot) {
            bestTimeTot = tempsTotal;
        }
    }

    printf("Fin de la session %s.\n", sessionName);
    printf("Meilleurs temps de la session %s : Secteur 1: %.3f, Secteur 2: %.3f, Secteur 3: %.3f, Total: %.3f\n\n",
           sessionName, bestTimeSec1, bestTimeSec2, bestTimeSec3, bestTimeTot);
    
    //static float outputArray[4];
    //outputArray[0] = bestTimeSec1;
    //outputArray[1] = bestTimeSec2;
    //outputArray[2] = bestTimeSec3;
    //outputArray[3] = bestTimeTot;
    //return outputArray;

}

int main() {
	// Corps du projet.
	// printf("n° voit | Temps S1 | Temps S2 | Temps S3 |  Total \n");
	// printf("--------|----------|----------|----------|---------\n");
	// printf("--------|bestTimeSec1|bestTimeSec2|bestTimeSec3|bestTimeTot\n");
	Essais();    

    int refSeed = time(NULL);

	qualifs("Q1", 18, refSeed++);
	qualifs("Q2", 15, refSeed++);
	qualifs("Q3", 12, refSeed++);

    //float* q1Results = qualifs("Q1", 18, refSeed++);
    //float* q2Results = qualifs("Q2", 15, refSeed++);
    //float* q3Results = qualifs("Q3", 12, refSeed++);

    //printf("Résultats Q1 : Secteur 1: %.3f, Secteur 2: %.3f, Secteur 3: %.3f, Total: %.3f\n", q1Results[0], q1Results[1], q1Results[2], q1Results[3]);
    //printf("Résultats Q2 : Secteur 1: %.3f, Secteur 2: %.3f, Secteur 3: %.3f, Total: %.3f\n", q2Results[0], q2Results[1], q2Results[2], q2Results[3]);
    //printf("Résultats Q3 : Secteur 1: %.3f, Secteur 2: %.3f, Secteur 3: %.3f, Total: %.3f\n", q3Results[0], q3Results[1], q3Results[2], q3Results[3]);


    return 0;
}


/* Idées :
	Un processus par voiture.
	Chaque process appelle GenRanNum trois fois → Génère trois temps par secteur.

	Valeurs pour l'état de la voiture :
	-1 → La voiture est crashée ;
	0 → La voiture est au stand ;
	1 → La voiture roule.

*/