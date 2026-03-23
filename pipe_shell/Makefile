.PHONY = all tidy-check format clean

all: pipe_shell

pipe_shell: pipe_shell.cpp
	clang++-19 -g3 -gdwarf-4 -Wall -std=c++2b -o pipe_shell pipe_shell.cpp

tidy-check: 
	clang-tidy-19 \
        --extra-arg=--std=c++2b \
        -warnings-as-errors=* \
        -header-filter=.* \
        pipe_shell.cpp

format:
	clang-format-19 -i --verbose --style=Chromium pipe_shell.cpp

clean:
	rm -f *.o pipe_shell
