CC = gcc
CFLAGS = -Wall -Wextra -g
SRC_DIR = src
INC_DIR = include
OBJ = $(SRC_DIR)/main.o \
      $(SRC_DIR)/car.o \
      $(SRC_DIR)/circuit.o \
      $(SRC_DIR)/csvutils.o \
      $(SRC_DIR)/race.o \
      $(SRC_DIR)/state.o \
      $(SRC_DIR)/utils.o
EXEC = f1_sim

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) -lm

$(SRC_DIR)/main.o: $(SRC_DIR)/main.c $(INC_DIR)/utils.h $(INC_DIR)/csvutils.h $(INC_DIR)/race.h $(INC_DIR)/state.h
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

$(SRC_DIR)/car.o: $(SRC_DIR)/car.c $(INC_DIR)/car.h $(INC_DIR)/utils.h
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

$(SRC_DIR)/circuit.o: $(SRC_DIR)/circuit.c $(INC_DIR)/circuit.h
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

$(SRC_DIR)/csvutils.o: $(SRC_DIR)/csvutils.c $(INC_DIR)/csvutils.h $(INC_DIR)/circuit.h
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

$(SRC_DIR)/race.o: $(SRC_DIR)/race.c $(INC_DIR)/race.h $(INC_DIR)/car.h $(INC_DIR)/circuit.h $(INC_DIR)/utils.h
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

$(SRC_DIR)/state.o: $(SRC_DIR)/state.c $(INC_DIR)/state.h $(INC_DIR)/race.h
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

$(SRC_DIR)/utils.o: $(SRC_DIR)/utils.c $(INC_DIR)/utils.h
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

clean:
	rm -f $(SRC_DIR)/*.o $(EXEC) gp_state.dat final_result.txt

.PHONY: all clean
