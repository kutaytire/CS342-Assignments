CXX       := gcc
CXX_FLAGS := -Wall -g

BIN := .
SRC := .

EXECUTABLE1 := proctopk
EXECUTABLE2 := threadtopk

all: clean $(BIN)/$(EXECUTABLE1) $(BIN)/$(EXECUTABLE2)

run: all
	clear
	./$(BIN)/$(EXECUTABLE1)
	./$(BIN)/$(EXECUTABLE2)

$(BIN)/$(EXECUTABLE1): $(SRC)/$(EXECUTABLE1).c $(SRC)/file_processor.c $(SRC)/freq_table.c
	@$(CXX) $(CXX_FLAGS) $^ -o $@

$(BIN)/$(EXECUTABLE2): $(SRC)/$(EXECUTABLE2).c $(SRC)/file_processor.c $(SRC)/freq_table.c
	@$(CXX) $(CXX_FLAGS) $^ -lpthread -o $@

clean:
	@-rm -rf $(SRC)/$(EXECUTABLE1) $(SRC)/$(EXECUTABLE1).o $(SRC)/$(EXECUTABLE2) $(SRC)/$(EXECUTABLE2).o *~

test: all
	clear
	@$(BIN)/$(EXECUTABLE1) 3 out-proctopk.txt 10 test/*.txt
	@cat ./out-proctopk.txt && echo "\n"
	@$(BIN)/$(EXECUTABLE2) 3 out-threadtopk.txt 10 test/*.txt
	@cat ./out-threadtopk.txt

valgrind: all
	clear
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes --show-reachable=yes --num-callers=500 --track-fds=yes --dsymutil=yes --child-silent-after-fork=yes --trace-children=yes -s $(BIN)/$(EXECUTABLE1) 3 out-proctopk.txt 10 test/*.txt
	@echo "\n"
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes --show-reachable=yes --num-callers=500 --track-fds=yes --dsymutil=yes --child-silent-after-fork=yes --trace-children=yes --fair-sched=try -s $(BIN)/$(EXECUTABLE2) 3 out-threadtopk.txt 10 test/*.txt
