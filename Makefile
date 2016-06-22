siod : siod.c lfsr.h
	g++ -std=c++11 -Wall -Werror -O2 siod.c -o siod -lsgutils2
clean:
	\rm -f siod
