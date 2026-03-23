.PHONY = clean all format tidy-check
TEST_CPPS = catch.cpp test_suite.cpp test_fsm.cpp
TEST_HPPS = catch.hpp

all: test_suite

FSM.o: FSM.cpp FSM.hpp
	clang++-19 -Wall -Wextra --std=c++2b -c FSM.cpp

ParseFSM.o: ParseFSM.cpp ParseFSM.hpp FSM.hpp
	clang++-19 -Wall -Wextra --std=c++2b -c $<

test_suite: catch.o test_suite.o test_fsm.o FSM.o ParseFSM.o
	clang++-19 -Wall -Wextra --std=c++2b -o $@ $^

test_suite.o: test_suite.cpp catch.hpp
	clang++-19 -Wall -Wextra --std=c++2b -c $<

test_fsm.o: test_fsm.cpp catch.hpp FSM.hpp ParseFSM.hpp
	clang++-19 -Wall -Wextra --std=c++2b -c $<

catch.o: catch.cpp catch.hpp
	clang++-19 -Wall -Wextra --std=c++2b -c $<

tidy-check: 
	clang-tidy-19 \
        --extra-arg=--std=c++2b \
        -warnings-as-errors=* \
        -header-filter=.* \
        $(filter-out $(TEST_CPPS), $(wildcard *.cpp))

format:
	clang-format-19 -i --verbose --style=Chromium \
		$(filter-out $(TEST_CPPS), $(wildcard *.cpp)) \
		$(filter-out $(TEST_HPPS), $(wildcard *.hpp))

clean:
	rm test_suite *.o
