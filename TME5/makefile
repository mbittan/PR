NOM = bittan
PRENOM = maxime
NUMTME = 5
CC = gcc
CFLAGS = -Wall
STUFF = -lrt -pthread
SRC = src
LIB = lib
INC = include
OBJ = obj
BIN = bin

all: directories ex1 ex2 ex3 ex4 ex5 ex6

directories: $(OBJ) $(BIN) $(LIB)

$(OBJ):
	mkdir -p $(OBJ)

$(BIN):
	mkdir -p $(BIN)

$(LIB):
	mkdir -p $(LIB)

ex1: $(BIN)/ex1

ex2: $(BIN)/ex2

ex3: $(BIN)/chat_client $(BIN)/chat_server

ex4: $(BIN)/ex4

ex5: $(BIN)/ex5

ex6: $(BIN)/ex6

#$(BIN)/ex1: $(OBJ)/ex1.o
#	$(CC) $(CFLAGS) -o $@ $^ $(STUFF)

#$(BIN)/ex2: $(OBJ)/ex2.o
#	$(CC) $(CFLAGS) -o $@ $^ $(STUFF)

#$(BIN)/ex3 : $(OBJ)/ex3.o
#	$(CC) $(CFLAGS) -o $@ $^ $(STUFF)

#$(BIN)/ex4 : $(OBJ)/ex4.o
#	$(CC) $(CFLAGS) -o $@ $^ $(STUFF)

#$(BIN)/ex5 :$(OBJ)/ex5.o
#	$(CC) $(CFLAGS) -o $@ $^ $(STUFF)

#$(BIN)/ex6 :$(OBJ)/ex6.o
#	$(CC) $(CFLAGS) -o $@ $^ $(STUFF)

$(BIN)/%: $(OBJ)/%.o
	$(CC) $(CFLAGS) -o $@ $^ $(STUFF)

$(OBJ)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -I$(INC) -c -o $@ $<

clean:
	rm -rf $(BIN) $(OBJ) $(LIB) $(SRC)/*~ $(INC)/*~

pack: clean
	cd .. && tar czfv $(NOM)-$(PRENOM).mi014.$(NUMTME).tgz TME$(NUMTME)

.PHONY: all clean pack ex1 ex2 ex3 ex4 ex5


