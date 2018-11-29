CC = mpicc -std=c11
FLAGS = -Wall -fopenmp -g
HEAD = project.h
OBJ_FILES = structures.o merging.o search.o main.o testing.o
OBJ_DIR = obj
EXE_DIR = exe
OBJ = $(OBJ_FILES:%=$(OBJ_DIR)/%)

project: $(OBJ)
	$(CC) $(FLAGS) -o $(EXE_DIR)/$@ $^

$(OBJ_DIR)/%.o: %.c $(HEAD)
	$(CC) $(FLAGS) -o $@ -c $<
