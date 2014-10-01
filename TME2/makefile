NOM = bittan
PRENOM = maxime
CC = gcc
CFLAGS = -Wall -pthread
SRC = src
LIB = lib
INC = include
OBJ = obj
BIN = bin

all: directories ex1 ex2 ex3 ex4-1 ex4-2 ex5

directories: $(OBJ) $(BIN) $(LIB)

$(OBJ):
	mkdir -p $(OBJ)

$(BIN):
	mkdir -p $(BIN)

$(LIB):
	mkdir -p $(LIB)

ex1: $(BIN)/ex1

ex2: $(BIN)/ex2

ex5: $(BIN)/ex5

ex3: $(BIN)/ex3

ex4-1: $(BIN)/ex4-1

ex4-1: $(BIN)/ex4-2


$(BIN)/ex1: $(OBJ)/ex1.o
	$(CC) $(CFLAGS) -o $@ $^ 

$(BIN)/ex2: $(OBJ)/ex2.o
	$(CC) $(CFLAGS) -o $@ $^

$(BIN)/ex5 : $(OBJ)/ex5.o
	$(CC) $(CFLAGS) -o $@ $^

$(BIN)/ex3 : $(OBJ)/ex3.o
	$(CC) $(CFLAGS) -o $@ $^

$(BIN)/ex4-1 : $(OBJ)/ex4-1.o
	$(CC) $(CFLAGS) -o $@ $^

$(BIN)/ex4-2 : $(OBJ)/ex4-2.o
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -I$(INC) -c -o $@ $<

clean:
	rm -rf $(BIN) $(OBJ) $(LIB) $(SRC)/*~ $(INC)/*~

pack: clean
	cd .. && tar czfv $(NOM)-$(PRENOM).mi014.2.tgz TME2

.PHONY: all clean pack ex1 ex2 ex3 ex4-1 ex4-2 ex5


