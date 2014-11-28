NOM = gouicem
PRENOM = redha
NUMTME = 7
CC = gcc
CFLAGS = -Wall -g
STUFF = 
SRC = src
LIB = lib
INC = include
OBJ = obj
BIN = bin

all: directories ex1 #ex2 ex3 ex4

directories: $(OBJ) $(BIN) $(LIB)

$(OBJ):
	mkdir -p $(OBJ)

$(BIN):
	mkdir -p $(BIN)

$(LIB):
	mkdir -p $(LIB)

ex1: $(BIN)/ftp_client $(BIN)/ftp_serveur

ex2: $(BIN)/recvfile $(BIN)/sendfile	

ex3: $(BIN)/env_client $(BIN)/env_serveur

ex4: $(BIN)/ftp_client $(BIN)/ftp_serveur

$(BIN)/ftp_serveur: $(OBJ)/ftp_serveur.o
	$(CC) $(CFLAGS) -o $@ $^ -pthread

$(BIN)/%: $(OBJ)/%.o
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -I$(INC) -c -o $@ $<

clean:
	rm -rf $(BIN) $(OBJ) $(LIB) $(SRC)/*~ $(INC)/*~
	rm -rf $(SRC)/*# $(INC)/*# $(SRC)/.#* $(INC)/.#*

pack: clean
	cd .. && tar czfv $(NOM)-$(PRENOM).mi014.$(NUMTME).tgz TME$(NUMTME)

.PHONY: all clean pack ex1 ex2 ex3 ex4


