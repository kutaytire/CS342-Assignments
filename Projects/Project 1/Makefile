CXX       := gcc
CXX_FLAGS := -Wall -g

BIN := .
SRC := .

EXECUTABLE1 := proctopk
EXECUTABLE2 := threadtopk

all: $(BIN)/$(EXECUTABLE1)

run: clean all
	clear
	./$(BIN)/$(EXECUTABLE1)

$(BIN)/$(EXECUTABLE1): $(SRC)/*.c
	$(CXX) $(CXX_FLAGS) $^ -o $@ $(LIBRARIES)

clean:
	@-rm -rf $(SRC)/$(EXECUTABLE1) $(SRC)/$(EXECUTABLE1).o *~

test: clean all
	clear
	$(BIN)/$(EXECUTABLE1) 3 out.txt 10 test/*.txt
	cat ./out.txt

valgrind: clean all
	clear
	@-rm -rf $(SRC)/memleak.txt
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes --show-reachable=yes --num-callers=500 --track-fds=yes --dsymutil=yes --child-silent-after-fork=yes --trace-children=yes -s $(BIN)/$(EXECUTABLE1) 3 out.txt 10 test/*.txt
