NOM = bittan
PRENOM = maxime
NUMTME = 5
CC = gcc
CFLAGS = -Wall -g
STUFF = -lrt -pthread
SRC = src
LIB = lib
INC = include
OBJ = obj
BIN = bin

all: directories ex1 ex2 ex3 ex4

directories: $(OBJ) $(BIN) $(LIB)

$(OBJ):
	mkdir -p $(OBJ)

$(BIN):
	mkdir -p $(BIN)

$(LIB):
	mkdir -p $(LIB)

ex1: $(BIN)/ex1

ex2: $(BIN)/ex2_aveccpt $(BIN)/ex2_sanscpt

ex3: $(BIN)/chat_client $(BIN)/chat_server

ex4: $(BIN)/ex4

$(BIN)/%: $(OBJ)/%.o
	$(CC) $(CFLAGS) -o $@ $^ $(STUFF)

$(OBJ)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -I$(INC) -c -o $@ $<

clean:
	rm -rf $(BIN) $(OBJ) $(LIB) $(SRC)/*~ $(INC)/*~
	rm -rf $(SRC)/*# $(INC)/*# $(SRC)/.#* $(INC)/.#*

pack: clean
	cd .. && tar czfv $(NOM)-$(PRENOM).mi014.$(NUMTME).tgz TME$(NUMTME)

.PHONY: all clean pack ex1 ex2 ex3 ex4


