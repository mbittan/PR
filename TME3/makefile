NOM = gouicem
PRENOM = redha
CC = gcc
CFLAGS = -Wall -pthread
SRC = src
LIB = lib
INC = include
OBJ = obj
BIN = bin

all: directories ex1 ex2 ex3 ex4 ex5

directories: $(OBJ) $(BIN) $(LIB)

$(OBJ):
	mkdir -p $(OBJ)

$(BIN):
	mkdir -p $(BIN)

$(LIB):
	mkdir -p $(LIB)

ex1: $(BIN)/ex1

ex2: $(BIN)/ex2

ex3: $(BIN)/ex3

ex4: $(BIN)/ex4

ex5: $(BIN)/ex5

$(BIN)/ex1: $(OBJ)/ex1.o
	$(CC) $(CFLAGS) -o $@ $^ 

$(BIN)/ex2: $(OBJ)/ex2.o
	$(CC) $(CFLAGS) -o $@ $^

$(BIN)/ex3 : $(OBJ)/thread_stack.o $(OBJ)/ex3.o
	$(CC) $(CFLAGS) -o $@ $^

$(BIN)/ex4 : $(OBJ)/thread_stack.o $(OBJ)/ex4.o
	$(CC) $(CFLAGS) -o $@ $^

$(BIN)/ex5 :$(OBJ)/ex5.o
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -I$(INC) -c -o $@ $<

clean:
	rm -rf $(BIN) $(OBJ) $(LIB) $(SRC)/*~ $(INC)/*~

pack: clean
	cd .. && tar czfv $(NOM)-$(PRENOM).mi014.3.tgz TME3

.PHONY: all clean pack ex1 ex2 ex3 ex4 ex5


