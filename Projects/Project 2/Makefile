CXX       := gcc
CXX_FLAGS := -Wall -g

BIN := .
SRC := .

EXECUTABLE1 := mps

all: clean $(BIN)/$(EXECUTABLE1)

run: all
	clear
	./$(BIN)/$(EXECUTABLE1)

$(BIN)/$(EXECUTABLE1): $(SRC)/main.c $(SRC)/queue.c $(SRC)/schedulers.c $(SRC)/util.c
	@$(CXX) $(CXX_FLAGS) $^ -lpthread -o $@

clean:
	@-rm -rf $(SRC)/$(EXECUTABLE1) $(SRC)/$(EXECUTABLE1).o *~
