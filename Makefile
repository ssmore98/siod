siod : siod.c lfsr.h
	g++ -static -std=c++11 -Wall -Werror -O2 siod.c -o siod -lsgutils2 -lz
clean:
	\rm -f siod
