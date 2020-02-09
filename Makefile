.PHONY: build run pack clean

# Compilator
CC = mpicc

# Flaguri compilator
CFLAGS = -Wall -Wextra -Wno-unused-parameter -std=c99

# Biblioteci externe
LIBS=-lm

# Sursa
SRC = processer.c

# Numele executabilului
EXE = processer

# Regula build
build: $(SRC)
	$(CC) $(CFLAGS) $^ -o $(EXE) $(LIBS)

# Regula de rulare
run: build
	mpirun -np $(TASKS) ./$(EXE) $(IN) $(OUT) $(FILTERS)

# Curatare director curent
clean:
	rm -f *.o $(EXE)
