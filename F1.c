#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

// Mettre à 1 pour activer les printf() de debug.
int debugPrints = 0;


// Prototype de fonction de conversion d'un nombre de secondes en un affichage mm' ss" mss (fonctionne pas car C comme Casse-Couilles avec les types de variables)
// On doit trop souvent faire une conversion d'un nombre de secondes en un affichage propre, d'où l'existence de cette fonction.
char timeFormat( float seconds ) {
	int timeMinutes = seconds / 60.0;
	int timeSeconds = seconds - (timeMinutes * 60);
	int timeMseconds = (seconds - (timeMinutes * 60) - timeSeconds) * 1000;
	char output[25];
	sprintf(output, "%d' %d\" %d\n", timeMinutes, timeSeconds, timeMseconds);
	return output;
}


float GenRanNum(int seedIncrementer, int secMin, int secMax) { //Génére un temps aléatoire compris entre secMin et secMan secondes.
	/*Se base sur le temps système pour générer une seed qui fournit une séquence de nombres aléatoires.
	seedIncrementer incrémente le srand pour éviter que la fonction aléatoire sorte systématiquement la même chose. */
	srand(time(NULL) + seedIncrementer);
	float randomTime = ((rand() % (((secMax - secMin)*1000) + 1) ) + (secMin*1000)) / 1000.0; //Donner un temps compris entre 25 et 45 secondes pour un secteur.
	return randomTime;
}

int doingLap(int *graine, float *tempsSec1, float *tempsSec2, float *tempsSec3, float *tempsTotal, float *trialsDuration) { //Appelée par une voiture → Lance une simulation d'un tour, il peut aboutir en temps, en un crash ou en pit.
	float bestTime = INFINITY;
	int seed = *graine;
	int diceResult = GenRanNum(seed++,1,101);
	if (debugPrints == 1) { printf("Valeur générée par le dé servant à déterminer l'aboutissement du tour: %d.\n", diceResult); }

	// Traitement de la chance que le pilote se plante.
	if ( diceResult == 100 ) { // Si le dé match le 1% de crash, on entame l'algorithme qui détermine à quel secteur l'incident est arrivé.
		int sectorResult = GenRanNum(seed++,1,100); // Pour déterminer dans quel secteur l'accident a eu lieu.
		if (debugPrints == 1) { printf("Valeur générée par le dé servant à déterminer la localisation du crash : %d.\n", sectorResult); }
		*tempsTotal = INFINITY; // Car dans tous les cas le tour ne sera pas terminé.
        // int pitState = 0;
		if ( sectorResult <= 33 ) { // Si crash au secteur 1 → Aucun secteur ne reçoit de temps
			if (debugPrints == 1) { printf("L'accident a eu lieu dans le secteur 1.\n\n"); }
			*tempsSec1 = INFINITY;
			*tempsSec2 = INFINITY;
			*tempsSec3 = INFINITY;
		}
		else if ( sectorResult <= 66 ) { // Si crash au secteur 2 → Seul secteur 1 reçoit un temps.
			if (debugPrints == 1) { printf("L'accident a eu lieu dans le secteur 2.\n\n"); }
			*tempsSec1 = GenRanNum(seed++, 25, 45); // Secteur 1 reçoit un temps.
			*tempsSec2 = INFINITY;
			*tempsSec3 = INFINITY;
		}
		else { // Si crash au secteur 3 → Seul secteur 3 ne reçoit pas de temps.
			if (debugPrints == 1) { printf("L'accident a eu lieu dans le secteur 3.\n\n"); }
			*tempsSec1 = GenRanNum(seed++, 25, 45); // Secteur 1 reçoit un temps.
			*tempsSec2 = GenRanNum(seed++, 25, 45); // Secteur 2 aussi.
			*tempsSec3 = INFINITY;
		}
	}

	// Traitement de la chance que le pilote retourne aux stands (else if car s'il ne s'est pas crashé, il est peut-être retourné aux stands)
	else if ( diceResult > 84 ) { // Si le résultat du dé est supérieur à 84 → Le pilote veut retourner aux stands.
		if (debugPrints == 1) { printf("Le pilote a décidé de retourner aux stands.\n"); }
		*tempsSec1 = GenRanNum(seed++, 25, 45); // Secteur 1 reçoit un temps.
		*tempsSec2 = GenRanNum(seed++, 25, 45); // Secteur 2 aussi.
		float remainingTrialsTime = 3600 - *trialsDuration;
		if (debugPrints == 1) { printf("Il reste %.3f secondes aux essais. Génération d'un temps compris entre 5 minutes et ce nombre de secondes...\n", remainingTrialsTime); }
		*tempsSec3 = GenRanNum(seed++, 300, remainingTrialsTime); // Il faut générer un temps aux stands. 3600 - temps qui est déjà passé -135 secondes

		*tempsTotal = *tempsSec1 + *tempsSec2 + *tempsSec3;
		if (debugPrints == 1) { printf("Temps passé aux stands: %f\n", *tempsSec3); }

		// Conversion temps total en écriture mm:ss:msc
		int tempsMinutes = *tempsTotal / 60.0;
		int tempsSecondes = *tempsTotal - (tempsMinutes * 60);
		int tempsMsecondes = (*tempsTotal - (tempsMinutes * 60) - tempsSecondes) * 1000;
		
		if (debugPrints == 1) { printf("Temps généré: %d:%d:%d = %.3f secondes\n\n", tempsMinutes, tempsSecondes, tempsMsecondes, *tempsTotal); }
	}
	else {
		if (debugPrints == 1) { printf("Tour normal.\n"); }
		*tempsSec1 = GenRanNum(seed++, 25, 45);
		*tempsSec2 = GenRanNum(seed++, 25, 45);
		*tempsSec3 = GenRanNum(seed++, 25, 45);

		// Conversion d'un total en secondes en un affichage min:sec:msec.
		// /!\ L'idée serait d'utiliser la fonction prototype timeFormat() quand elle fonctionne.
		*tempsTotal = *tempsSec1 + *tempsSec2 + *tempsSec3;
		int tempsMinutes = *tempsTotal / 60.0;
		int tempsSecondes = *tempsTotal - (tempsMinutes * 60);
		int tempsMsecondes = (*tempsTotal - (tempsMinutes * 60) - tempsSecondes) * 1000;
		
		if (debugPrints == 1) { printf("Temps généré: %d:%d:%d = %.3f secondes\n\n", tempsMinutes, tempsSecondes, tempsMsecondes, *tempsTotal); }
	}
	*graine = seed;
	//int outputArray[] = {tempsSec1, tempsSec2, tempsSec3, tempsTotal};
	//return outputArray;
}


int Essais() {
	// Variables qui contiendront les meilleurs temps.
	float bestTimeSec1 = INFINITY;
	float bestTimeSec2 = INFINITY;
	float bestTimeSec3 = INFINITY;
	float bestTimeTot = INFINITY;
	
	// Variables qui seront mises à jour par doingLap() à chaque fois qu'une voiture effectue un tour.
	float tempsSec1, tempsSec2, tempsSec3, tempsTotal;
	int seed = 0;
	
	
	int seed4GenRanTime = 0; // Utilisée comme graine par GenRanNum() et incrémentée à chaque utilisation.
	float trialsDuration = 0; // Variable qui compte la progression temporelle des essais.
	if (GenRanNum(seed4GenRanTime++, 0, 100) < 20) { // La voiture décide-t-elle de commencer plus tard que le commencement des essais ?
		trialsDuration = GenRanNum(seed4GenRanTime++, 0, 40); // De combien de temps ? REVENIR DESSUS
	}
	
	while ( trialsDuration < 3600 ) { // Boucle tournant tant que les essais n'ont pas atteint 60 minutes.
		doingLap(&seed, &tempsSec1, &tempsSec2, &tempsSec3, &tempsTotal, &trialsDuration); // Lancement d'un tour de voiture.
		if (debugPrints == 1) { printf("Valeurs des meilleurs temps: %.3f, %.3f, %.3f, %.3f.\n", bestTimeSec1, bestTimeSec2, bestTimeSec3, bestTimeTot); } // Pas beau, faut faire un meilleur affichage.
		if (debugPrints == 1) { printf("Temps effectués par la voiture: %.3f, %.3f, %.3f, %.3f. duree essais %.3f \n", tempsSec1, tempsSec2, tempsSec3, tempsTotal, trialsDuration); } // Idem.
		trialsDuration += tempsTotal; // Ajout du temps total effectué par la voiture sur son tour au temps des essais.
		
		// Mise à jour des meilleurs temps.
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
	// Classement final
	printf(" Pos. | Temps S1 | Temps S2 | Temps S3 |  Total   \n");
	printf("------|----------|----------|----------|----------\n");
	// L'idée ici sera d'insérer les autres voitures.
	printf("------|----------|----------|----------|----------\n");
	printf(" BEST |  %.3f  |  %.3f  |  %.3f  |  %.3f  \n", bestTimeSec1, bestTimeSec2, bestTimeSec3, bestTimeTot); // Faudra convertir l'affichage des temps avec la fonction timeFormat()
}

int courseDimanche() {
		// Variables qui contiendront les meilleurs temps.
	float bestTimeSec1 = INFINITY;
	float bestTimeSec2 = INFINITY;
	float bestTimeSec3 = INFINITY;
	float bestTimeTot = INFINITY;
	
	// Variables qui seront mises à jour par doingLap() à chaque fois qu'une voiture effectue un tour.
	float tempsSec1, tempsSec2, tempsSec3, tempsTotal;
	int seed = 0;
	
	int seed4GenRanTime = 0; // Utilisée comme graine par GenRanNum() et incrémentée à chaque utilisation.
	int tourTournoi = 0;
	int tourCircuit = 100; // A modifier : Aller chercher la taille du tour dans le fichier circuit
	int tourTournoiMax = 300/tourCircuit;

	while(tourTournoi < tourTournoiMax) {
		doingLap(&seed, &tempsSec1, &tempsSec2, &tempsSec3, &tempsTotal, &tourTournoi);
		tourTournoi++;

	}

	// Mise à jour des meilleurs temps.
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

	// Classement final
	printf(" Pos. | Temps S1 | Temps S2 | Temps S3 |  Total   \n");
	printf("------|----------|----------|----------|----------\n");
	// L'idée ici sera d'insérer les autres voitures.
	printf("------|----------|----------|----------|----------\n");
	printf(" BEST |  %.3f  |  %.3f  |  %.3f  |  %.3f  \n", bestTimeSec1, bestTimeSec2, bestTimeSec3, bestTimeTot); // Faudra convertir l'affichage des temps avec la fonction timeFormat()
}
int courseSprint() {
			// Variables qui contiendront les meilleurs temps.
	float bestTimeSec1 = INFINITY;
	float bestTimeSec2 = INFINITY;
	float bestTimeSec3 = INFINITY;
	float bestTimeTot = INFINITY;
	
	// Variables qui seront mises à jour par doingLap() à chaque fois qu'une voiture effectue un tour.
	float tempsSec1, tempsSec2, tempsSec3, tempsTotal;
	int seed = 0;
	
	int seed4GenRanTime = 0; // Utilisée comme graine par GenRanNum() et incrémentée à chaque utilisation.
	int tourTournoi = 0;
	int tourCircuit = 100; // A modifier : Aller chercher la taille du tour dans le fichier circuit
	int tourTournoiMax = 100/tourCircuit;
	while(tourTournoi < tourTournoiMax) {
		doingLap(&seed, &tempsSec1, &tempsSec2, &tempsSec3, &tempsTotal, &tourTournoi);
		tourTournoi++;
	}

	// Mise à jour des meilleurs temps.
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

	// Classement final
	printf(" Pos. | Temps S1 | Temps S2 | Temps S3 |  Total   \n");
	printf("------|----------|----------|----------|----------\n");
	// L'idée ici sera d'insérer les autres voitures.
	printf("------|----------|----------|----------|----------\n");
	printf(" BEST |  %.3f  |  %.3f  |  %.3f  |  %.3f  \n", bestTimeSec1, bestTimeSec2, bestTimeSec3, bestTimeTot); // Faudra convertir l'affichage des temps avec la fonction timeFormat()

}

int main() {
	// Corps du projet.
	// printf("n° voit | Temps S1 | Temps S2 | Temps S3 |  Total \n");
	// printf("--------|----------|----------|----------|---------\n");
	// printf("--------|bestTimeSec1|bestTimeSec2|bestTimeSec3|bestTimeTot\n");
	printf("ESSAIS P1\n");
	sleep(1);
	courseDimanche();
	printf("\n\n\n");

	printf("ESSAIS P2\n");
	sleep(1);
	courseDimanche();
	printf("\n\n\n");

	printf("ESSAIS P3\n");
	sleep(1);
	courseDimanche();
	printf("\n\n\n");
}