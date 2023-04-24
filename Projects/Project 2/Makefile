CXX       := gcc
CXX_FLAGS := -Wall -g

BIN := .
SRC := .

EXECUTABLE1 := mps
EXECUTABLE2 := mps_cv

all: clean $(BIN)/$(EXECUTABLE1) $(BIN)/$(EXECUTABLE2)

run: all
	clear
	./$(BIN)/$(EXECUTABLE1)
	./$(BIN)/$(EXECUTABLE2)

$(BIN)/$(EXECUTABLE1): $(SRC)/mps.c $(SRC)/queue.c $(SRC)/schedulers.c $(SRC)/util.c
	@$(CXX) $(CXX_FLAGS) $^ -lpthread -lm -o $@

$(BIN)/$(EXECUTABLE2): $(SRC)/mps_cv.c $(SRC)/queue.c $(SRC)/schedulers_cv.c $(SRC)/util.c
	@$(CXX) $(CXX_FLAGS) $^ -lpthread -lm -o $@

clean:
	@-rm -rf $(SRC)/$(EXECUTABLE1) $(SRC)/$(EXECUTABLE1).o $(SRC)/$(EXECUTABLE2) $(SRC)/$(EXECUTABLE2).o *~
