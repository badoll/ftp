SRC = .
OBJ = obj
CPP11 = -std=c++11
BINCLIENT = bin/client
BINSERVER = bin/server
OBJECT_CLIENT = $(OBJ)/client.o $(OBJ)/client_connect.o 
OBJECT_SERVER = $(OBJ)/server.o $(OBJ)/server_connect.o 
OBJECT_COMMON = $(OBJ)/common.o

CLIENT = $(BINCLIENT)/client
SERVER = $(BINSERVER)/server

.PHONY: all
all: client server

client: $(OBJECT_CLIENT) $(OBJECT_COMMON)
	g++ $(CPP11) $^ -o $(CLIENT) 

server: $(OBJECT_SERVER) $(OBJECT_COMMON)
	g++ $(CPP11) $^ -o $(SERVER)

$(OBJECT_COMMON): $(SRC)/common.h
$(OBJECT_CLIENT): $(SRC)/client.h
$(OBJECT_SERVER): $(SRC)/server.h

$(OBJ)/%.o: $(SRC)/%.cpp
	g++ $(CPP11) -c $< -o $@


.PHONY: clean
clean:
	rm -f $(OBJ)/*.o
	rm -f $(CLIENT)
	rm -f $(SERVER)
