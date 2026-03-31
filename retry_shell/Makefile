all: roulette counter stdin_echo nap retry_shell

retry_shell: retry_shell.cpp
	clang++-19 -g3 -gdwarf-4 -Wall --std=c++2b -o retry_shell retry_shell.cpp

counter: counter.cpp
	clang++-19 -g3 -gdwarf-4 -Wall --std=c++2b -o counter counter.cpp

roulette: roulette.cpp
	clang++-19 -g3 -gdwarf-4 -Wall --std=c++2b -o roulette roulette.cpp

stdin_echo: stdin_echo.cpp
	clang++-19 -g3 -gdwarf-4 -Wall --std=c++2b -o stdin_echo stdin_echo.cpp

nap: nap.cpp
	clang++-19 -g3 -gdwarf-4 -Wall --std=c++2b -o nap nap.cpp

tidy-check: 
	clang-tidy-19 \
        --extra-arg=--std=c++2b \
        -warnings-as-errors=* \
        -header-filter=.* \
        retry_shell.cpp

format:
	clang-format-19 -i --verbose --style=Chromium retry_shell.cpp

clean:
	rm -f roulette counter stdin_echo nap retry_shell
