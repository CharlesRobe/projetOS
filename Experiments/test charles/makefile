CC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude
SRC_DIR = src
INC_DIR = include

OBJ = $(SRC_DIR)/main.o \
      $(SRC_DIR)/car.o \
      $(SRC_DIR)/circuit.o \
      $(SRC_DIR)/csvutils.o \
      $(SRC_DIR)/f1shared.o \
      $(SRC_DIR)/race.o \
      $(SRC_DIR)/rwcourtois.o \
      $(SRC_DIR)/utils.o

EXEC = F1

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) -lm

$(SRC_DIR)/main.o: $(SRC_DIR)/main.c $(INC_DIR)/f1shared.h $(INC_DIR)/race.h $(INC_DIR)/rwcourtois.h $(INC_DIR)/utils.h
	$(CC) $(CFLAGS) -c $< -o $@

$(SRC_DIR)/car.o: $(SRC_DIR)/car.c $(INC_DIR)/car.h $(INC_DIR)/f1shared.h $(INC_DIR)/rwcourtois.h $(INC_DIR)/utils.h
	$(CC) $(CFLAGS) -c $< -o $@

$(SRC_DIR)/circuit.o: $(SRC_DIR)/circuit.c $(INC_DIR)/circuit.h
	$(CC) $(CFLAGS) -c $< -o $@

$(SRC_DIR)/csvutils.o: $(SRC_DIR)/csvutils.c $(INC_DIR)/csvutils.h $(INC_DIR)/circuit.h
	$(CC) $(CFLAGS) -c $< -o $@

$(SRC_DIR)/f1shared.o: $(SRC_DIR)/f1shared.c $(INC_DIR)/f1shared.h $(INC_DIR)/utils.h
	$(CC) $(CFLAGS) -c $< -o $@

$(SRC_DIR)/race.o: $(SRC_DIR)/race.c $(INC_DIR)/race.h $(INC_DIR)/f1shared.h $(INC_DIR)/car.h $(INC_DIR)/rwcourtois.h $(INC_DIR)/utils.h
	$(CC) $(CFLAGS) -c $< -o $@

$(SRC_DIR)/rwcourtois.o: $(SRC_DIR)/rwcourtois.c $(INC_DIR)/rwcourtois.h $(INC_DIR)/utils.h
	$(CC) $(CFLAGS) -c $< -o $@

$(SRC_DIR)/utils.o: $(SRC_DIR)/utils.c $(INC_DIR)/utils.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(SRC_DIR)/*.o $(EXEC)

.PHONY: all clean
