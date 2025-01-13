# Variables
CC = gcc
CFLAGS = -Wall -Wextra -pedantic
SRC = main.c f1_race.c qualif.c essais.c
OBJ = $(SRC:.c=.o)
EXEC = projet

# Règle par défaut : compiler l'exécutable
all: $(EXEC)

# Règle pour générer l'exécutable
$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC) $(CFLAGS)

# Règle pour compiler chaque fichier source en .o
%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

# Nettoyer les fichiers objets et l'exécutable
clean:
	rm -f $(OBJ) $(EXEC)

# Supprimer les fichiers objets uniquement
clean-obj:
	rm -f $(OBJ)

