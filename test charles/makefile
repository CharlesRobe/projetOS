CC = gcc
CFLAGS = -Wall -Wextra -g
SRC = src
INC = include
OBJ = $(SRC)/main.o \
      $(SRC)/car.o \
      $(SRC)/circuit.o \
      $(SRC)/csvutils.o \
      $(SRC)/race.o \
      $(SRC)/state.o \
      $(SRC)/utils.o

EXEC = f1_sim

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) -lm

$(SRC)/main.o: $(SRC)/main.c $(INC)/utils.h $(INC)/csvutils.h $(INC)/race.h $(INC)/state.h
	$(CC) $(CFLAGS) -I$(INC) -c $< -o $@

$(SRC)/car.o: $(SRC)/car.c $(INC)/car.h
	$(CC) $(CFLAGS) -I$(INC) -c $< -o $@

$(SRC)/circuit.o: $(SRC)/circuit.c $(INC)/circuit.h
	$(CC) $(CFLAGS) -I$(INC) -c $< -o $@

$(SRC)/csvutils.o: $(SRC)/csvutils.c $(INC)/csvutils.h $(INC)/circuit.h
	$(CC) $(CFLAGS) -I$(INC) -c $< -o $@

$(SRC)/race.o: $(SRC)/race.c $(INC)/race.h $(INC)/car.h $(INC)/circuit.h $(INC)/utils.h
	$(CC) $(CFLAGS) -I$(INC) -c $< -o $@

$(SRC)/state.o: $(SRC)/state.c $(INC)/state.h $(INC)/race.h
	$(CC) $(CFLAGS) -I$(INC) -c $< -o $@

$(SRC)/utils.o: $(SRC)/utils.c $(INC)/utils.h
	$(CC) $(CFLAGS) -I$(INC) -c $< -o $@

clean:
	rm -f $(SRC)/*.o $(EXEC) gp_state.dat final_result.txt

.PHONY: all clean
