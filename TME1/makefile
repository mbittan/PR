NOM = bittan
PRENOM = maxime
CC = gcc
CFLAGS = -Wall
SRC = src
LIB = lib
INC = include
OBJ = obj
BIN = bin

all: directories max_calcul nfork ex3 ex4 ex5 ex6 ex7

directories: $(OBJ) $(BIN) $(LIB)

$(OBJ):
	mkdir -p $(OBJ)

$(BIN):
	mkdir -p $(BIN)

$(LIB):
	mkdir -p $(LIB)

max_calcul : $(BIN)/max_calcul

nfork: $(BIN)/nfork

ex3: $(BIN)/ex3

ex4: $(BIN)/ex4

ex5: $(BIN)/ex5

ex6: $(BIN)/ex6

ex7: $(BIN)/ex7

$(BIN)/nfork : $(OBJ)/nfork.o
	$(CC) $(CFLAGS) -o $@ $^

$(BIN)/max_calcul: $(OBJ)/max_calcul.o $(OBJ)/max_func.o
	$(CC) $(CFLAGS) -o $@ $^

$(BIN)/ex3: $(OBJ)/ex3.o
	$(CC) $(CFLAGS) -o $@ $^

$(BIN)/ex4: $(OBJ)/ex4.o
	$(CC) $(CFLAGS) -o $@ $^

$(BIN)/ex5 : $(OBJ)/ex5.o
	$(CC) $(CFLAGS) -o $@ $^

$(BIN)/ex6 : $(OBJ)/ex6.o
	$(CC) $(CFLAGS) -o $@ $^

$(BIN)/ex7 : $(OBJ)/ex7.o
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ)/max_func.o: $(SRC)/max_func.c $(INC)/max_func.h
	$(CC) $(CFLAGS) -I$(INC) -c -o $@ $<

$(OBJ)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -I$(INC) -c -o $@ $<

clean:
	rm -rf $(BIN) $(OBJ) $(LIB) $(SRC)/*~ $(INC)/*~

pack: clean
	cd .. && tar czfv $(NOM)-$(PRENOM).mi014.1.tgz TME1

.PHONY: all clean max_calcul nfork ex4 ex3 ex5 ex6 ex7 pack


