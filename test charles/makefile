CC = gcc
CFLAGS = -Wall -Wextra -g

# Liste des fichiers .c à compiler
SRC = src
INC = include

OBJ = $(SRC)/main.o \
      $(SRC)/car.o \
      $(SRC)/circuit.o \
      $(SRC)/csvutils.o \
      $(SRC)/f1shared.o \
      $(SRC)/race.o \
      $(SRC)/rwcourtois.o \
      $(SRC)/state.o \
      $(SRC)/utils.o

EXEC = f1_courtois_final

# Cible principale
all: $(EXEC)

# Edition de liens
$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) -lm

# Règles de compilation (chacune dépend de ses .h)
$(SRC)/main.o: $(SRC)/main.c $(INC)/car.h $(INC)/circuit.h $(INC)/csvutils.h $(INC)/f1shared.h $(INC)/race.h $(INC)/rwcourtois.h $(INC)/utils.h
	$(CC) $(CFLAGS) -I$(INC) -c $< -o $@

$(SRC)/car.o: $(SRC)/car.c $(INC)/car.h $(INC)/f1shared.h $(INC)/rwcourtois.h $(INC)/utils.h
	$(CC) $(CFLAGS) -I$(INC) -c $< -o $@

$(SRC)/circuit.o: $(SRC)/circuit.c $(INC)/circuit.h
	$(CC) $(CFLAGS) -I$(INC) -c $< -o $@

$(SRC)/csvutils.o: $(SRC)/csvutils.c $(INC)/csvutils.h $(INC)/circuit.h $(INC)/utils.h
	$(CC) $(CFLAGS) -I$(INC) -c $< -o $@

$(SRC)/f1shared.o: $(SRC)/f1shared.c $(INC)/f1shared.h $(INC)/utils.h
	$(CC) $(CFLAGS) -I$(INC) -c $< -o $@

$(SRC)/race.o: $(SRC)/race.c $(INC)/race.h $(INC)/f1shared.h $(INC)/car.h $(INC)/rwcourtois.h $(INC)/utils.h
	$(CC) $(CFLAGS) -I$(INC) -c $< -o $@

$(SRC)/rwcourtois.o: $(SRC)/rwcourtois.c $(INC)/rwcourtois.h $(INC)/utils.h
	$(CC) $(CFLAGS) -I$(INC) -c $< -o $@

$(SRC)/state.o: $(SRC)/state.c $(INC)/state.h
	$(CC) $(CFLAGS) -I$(INC) -c $< -o $@

$(SRC)/utils.o: $(SRC)/utils.c $(INC)/utils.h
	$(CC) $(CFLAGS) -I$(INC) -c $< -o $@

# Nettoyage
clean:
	rm -f $(SRC)/*.o $(EXEC)

.PHONY: all clean
