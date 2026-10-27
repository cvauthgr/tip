main : main.cpp
	g++ -std=c++23 -Wall -Wextra -Wpedantic -Werror -O0 -g3 -march=native main.cpp -o main

run :
	./main

valgrind :
	valgrind -s ./main

debug :
	~/raddebugger/build/raddbg ./main

PHONY. = run valgrind debug
