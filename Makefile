all: folder alice bob

GCC = gcc
BIN = bin
OBJ = obj
INC = -Iinc/ -w

bob: folder cripto socket des rsa
	+@echo "Compile: $@"
	@$(GCC) src/$@.c $(INC) -c -o $(OBJ)/$@.o -lm -lssl -lcrypto

	@echo "Linking..."
	+@$(GCC) -o $(BIN)/appBob $(OBJ)/bob.o $(OBJ)/rsa.o $(OBJ)/des.o $(OBJ)/cripto.o $(OBJ)/socket.o -lm -lssl -lcrypto
	@echo "Binary are ready in $(BIN)/appBob"

alice: folder cripto socket des rsa
	+@echo "Compile: $@"
	@$(GCC) src/$@.c $(INC) -c -o $(OBJ)/$@.o -lm -lssl -lcrypto

	@echo "Linking..."
	+@$(GCC) -o $(BIN)/appAlice $(OBJ)/alice.o $(OBJ)/rsa.o $(OBJ)/des.o $(OBJ)/cripto.o $(OBJ)/socket.o -lm -lssl -lcrypto
	@echo "Binary are ready in $(BIN)/appAlice"
rsa:
	+@echo "Compile: $@"
	@$(GCC) src/$@.c $(INC) -c -o $(OBJ)/$@.o

des:
	+@echo "Compile: $@"
	@$(GCC) src/$@.c $(INC) -c -o $(OBJ)/$@.o -lm -lssl -lcrypto

socket:
	+@echo "Compile: $@"
	@$(GCC) src/$@.c $(INC) -c -o $(OBJ)/$@.o -lm

cripto:
	+@echo "Compile: $@"
	@$(GCC) src/$@.c $(INC) -c -o $(OBJ)/$@.o -lm

folder:
	@mkdir -p $(OBJ) $(BIN)

clean:
	@rm -rf $(OBJ)/* $(BIN)/* $(OBJ) $(BIN)
