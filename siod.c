#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

void print_usage(char *p) {
	fprintf(stderr, "Usage:\n\n%s [rw] [rs] [0-9]+ [0-9]+ [0123] /dev/sg[0-9]+ <logfile>\n", p);
	exit(-2);
}

/*
 * Exit codes:
 * 0  : Normal Termination
 * -1 : caught SIGTERM
 * -2 : error in command line parameters
 */

int main(int argc, char **argv) {
	if (argc != 8) {
		print_usage(argv[0]);
	}
	bool is_write = false;
	switch (argv[1][0]) {
		case 'R':
		case 'r':
			is_write = false;
			break;
		case 'W':
		case 'w':
			is_write = true;
			break;
		default:
		       	print_usage(argv[0]);
	}
	bool is_random = false;
	switch (argv[2][0]) {
		case 'R':
		case 'r':
			is_random = true;
			break;
		case 'S':
		case 's':
			is_random = false;
			break;
		default:
		       	print_usage(argv[0]);
	}
	uint16_t iosize = 0;
	if (1 != sscanf(argv[3], "%hu", &iosize) || !iosize) {
	       	print_usage(argv[0]);
	}
	uint16_t qdepth = 0;
	if (1 != sscanf(argv[4], "%hu", &qdepth) || !qdepth || (qdepth > 128)) {
	       	print_usage(argv[0]);
	}
	uint8_t key = 0;
	switch (argv[5][0]) {
		case '0':
			key = 0;
			break;
		case '1':
			key = 1;
			break;
		case '2':
			key = 2;
			break;
		case '3':
			key = 3;
			break;
		default:
		       	print_usage(argv[0]);
	}
	const char * const device = argv[6];
	const char * const logfilename = argv[7];
	return 0;
}
