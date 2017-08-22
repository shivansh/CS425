CC=	gcc
SRC=	./src
OBJ=	./obj
INCLUDE=./include
CFLAGS=	-I$(INCLUDE)
_DEPS = standard.h
DEPS=	$(patsubst %, $(INCLUDE)/%, $(_DEPS))

all:
	make server
	make client

$(OBJ)/%.o: $(SRC)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

server: $(OBJ)/server.o
	$(CC) -o server $(OBJ)/server.o $(CFLAGS)

client: $(OBJ)/client.o
	$(CC) -o client $(OBJ)/client.o $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(OBJ)/*
	rm -f server client
