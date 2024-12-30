CC = gcc
CFLAGS = -Wall -Wextra -g
SRC_DIR = src
INC_DIR = include
OBJ = $(SRC_DIR)/main.o \
      $(SRC_DIR)/car.o  \
      $(SRC_DIR)/race.o \
      $(SRC_DIR)/utils.o
EXEC = f1_sim

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

$(SRC_DIR)/main.o: $(SRC_DIR)/main.c $(INC_DIR)/race.h $(INC_DIR)/utils.h
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

$(SRC_DIR)/car.o: $(SRC_DIR)/car.c $(INC_DIR)/car.h $(INC_DIR)/utils.h
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

$(SRC_DIR)/race.o: $(SRC_DIR)/race.c $(INC_DIR)/race.h $(INC_DIR)/car.h $(INC_DIR)/utils.h
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

$(SRC_DIR)/utils.o: $(SRC_DIR)/utils.c $(INC_DIR)/utils.h
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

clean:
	rm -f $(SRC_DIR)/*.o $(EXEC) results_stage_*.txt

.PHONY: all clean
