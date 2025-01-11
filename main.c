#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "f1_race.h"
#define CSV_FILENAME "circuits.csv"
#define ETAT_FILENAME "etat"
#define MAX_LINE 256

/* ---------------------------------------------------------------------------
 * Prototypes
 * --------------------------------------------------------------------------- */
int circuit_exists_in_csv(const char *circuit_name, int *circuit_length);
void handle_no_argument(void);
int create_circuit_folder(const char *circuit_name, int circuit_length);
int update_etat_on_creation(const char *circuit_name, int circuit_state);

int main(int argc, char *argv[])
{
    // CAS 1 : AUCUN ARGUMENT => on traite le fichier "etat"
    if (argc == 1) {
        handle_no_argument();
        return 0;
    }

    // CAS 2 : AVEC ARGUMENT => "./projet <NomCircuit>"
    const char *circuit_name = argv[1];

    // 1) Vérifier si un dossier portant le nom du circuit existe déjà
    struct stat st;
    if (stat(circuit_name, &st) == 0 && S_ISDIR(st.st_mode)) {
        fprintf(stderr, "Erreur : Le dossier '%s' existe déjà. Veuillez continuez celui en cours avec ./projet ou en relancer un autre.\n", circuit_name);
        return 1;
    }

    // 2) Vérifier si le circuit est dans le CSV
    int circuit_length = 0;
    if (!circuit_exists_in_csv(circuit_name, &circuit_length)) {
        fprintf(stderr, "Erreur : Le circuit '%s' n'est pas présent dans '%s'.\n",
                circuit_name, CSV_FILENAME);
        return 1;
    }

    // 3) Créer le dossier et le fichier "taillecircuit.txt"
    if (create_circuit_folder(circuit_name, circuit_length) != 0) {
        fprintf(stderr, "Erreur lors de la création du dossier '%s'.\n", circuit_name);
        return 1;
    }

    // 4) Mettre à jour "etat" pour signaler qu'on a commencé ce circuit à l'état 1
    if (update_etat_on_creation(circuit_name, 1) != 0) {
        fprintf(stderr, "Erreur lors de la mise à jour de '%s'.\n", ETAT_FILENAME);
        return 1;
    }

    // 5) Indiquer qu'on lance la toute première étape (état=1 => Essais)
    printf("Debut %s. Lancement de la phase d'essais n°1.\n",
           circuit_name);

    return 0;
}

/* ---------------------------------------------------------------------------
 * CAS : ./projet (sans argument)
 * Lit le fichier "etat" et agit en fonction de la valeur lue :
 *   - état < 3  => Essais
 *   - 3 <= état < 6 => Qualifications
 *   - état = 6 => Course
 * On incrémente la valeur de l'état après chaque phase.
 * --------------------------------------------------------------------------- */
void handle_no_argument(void)
{
    FILE *f = fopen(ETAT_FILENAME, "r+");
    if (!f) {
        fprintf(stderr, "Erreur : Le fichier '%s' est introuvable. Aucun circuit en cours.\n", ETAT_FILENAME);
        return;
    }

    char current_circuit[128] = "";
    int state = 0;

    // Lire la première ligne (nom du circuit)
    if (!fgets(current_circuit, sizeof(current_circuit), f)) {
        fprintf(stderr, "Erreur : Le fichier '%s' est vide ou mal formé. Aucun circuit en cours.\n", ETAT_FILENAME);
        fclose(f);
        return;
    }

    // Supprimer le retour à la ligne de la fin (si présent)
    current_circuit[strcspn(current_circuit, "\n")] = '\0';

    // Lire la deuxième ligne (état numérique)
    if (fscanf(f, "%d", &state) != 1) {
        fprintf(stderr, "Erreur : L'état du circuit est manquant ou mal formé dans '%s'.\n", ETAT_FILENAME);
        fclose(f);
        return;
    }

    // Afficher le circuit en cours et l'état
    printf("Circuit en cours : %s | État actuel : %d\n", current_circuit, state);

    // Selon la valeur de state, on lance la phase correspondante
    if (state < 3) {
        // Essais
        printf("Lancement des Essais n° %d pour le circuit '%s'.\n",state+1, current_circuit);
    } else if (state < 6) {
        // Qualifications
        printf("Lancement des Qualifications n° %d pour le circuit '%s'.\n", state-2,current_circuit);
    } else if (state == 6)  {
        // Course
        printf("Lancement de la Course pour le circuit '%s'.\n", current_circuit);
        f1_race_run(current_circuit);
        if (remove("etat") == 0) {  
          printf("Le fichier 'etat' a été supprimé avec succès.\n");
          } else {
              perror("Erreur lors de la suppression du fichier 'etat'");
          }

    }
    

    // Incrémenter l'état
    state++;

    // Réécrire dans etat
    rewind(f);
    fprintf(f, "%s\n%d\n", current_circuit, state);

    fclose(f);

  
}

/* ---------------------------------------------------------------------------
 * Fonction qui vérifie si "circuit_name" se trouve dans "circuits.csv".
 * Si trouvé, on remplit "circuit_length" et on retourne 1.
 * Sinon on retourne 0.
 * --------------------------------------------------------------------------- */
int circuit_exists_in_csv(const char *circuit_name, int *circuit_length)
{
    FILE *file = fopen(CSV_FILENAME, "r");
    if (!file) {
        perror("Erreur lors de l'ouverture du fichier CSV");
        return 0;
    }

    char line[MAX_LINE];
    while (fgets(line, sizeof(line), file)) {
        // Supprime le \n à la fin
        line[strcspn(line, "\n")] = '\0';

        int circuit_num = 0;
        float length_f = 0.0;
        char name[128] = "";

        // Ex: "6,3.337,Monaco,Circuit de Monaco"
        // On veut extraire (int circuit_num), (float length_f), pays (ignoré), nom (char name)
        if (sscanf(line, "%d,%f,%*[^,],%127[^\n]", &circuit_num, &length_f, name) == 3) {
            // Comparaison insensible à la casse
            if (strcasecmp(name, circuit_name) == 0) {
                *circuit_length = (int)(length_f * 1000); // Convertir en mètres
                fclose(file);
                return 1; // trouvé
            }
        }
    }

    fclose(file);
    return 0; // non trouvé
}

/* ---------------------------------------------------------------------------
 * Créer un dossier portant le nom du circuit, puis y créer un fichier
 * "taillecircuit.txt" qui contient la taille du circuit.
 * --------------------------------------------------------------------------- */
int create_circuit_folder(const char *circuit_name, int circuit_length)
{
    if (mkdir(circuit_name, 0755) != 0) {
        perror("Erreur mkdir");
        return -1;
    }

    char filename[256];
    snprintf(filename, sizeof(filename), "%s/taillecircuit.txt", circuit_name);

    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("Erreur fopen");
        return -1;
    }
    fprintf(f, "%d\n", circuit_length);
    fclose(f);

    return 0;
}

/* ---------------------------------------------------------------------------
 * Mettre à jour le fichier "etat" pour signaler qu'un nouveau circuit
 * débute à l'état demandé (par ex. 1).
 * --------------------------------------------------------------------------- */
int update_etat_on_creation(const char *circuit_name, int circuit_state)
{
    FILE *f = fopen(ETAT_FILENAME, "w");
    if (!f) {
        perror("Erreur fopen etat");
        return -1;
    }

    // Écrire le nom du circuit (ligne 1) et l'état initial (ligne 2)
    fprintf(f, "%s\n%d\n", circuit_name, circuit_state);

    fclose(f);
    return 0;
}

