#include "utils.h"

// Fonction d'erreur
void error_exit(const char *message)
{
    perror(message);
    exit(EXIT_FAILURE);
}
