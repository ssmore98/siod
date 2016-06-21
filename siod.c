#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <scsi/sg_cmds.h>
#include <scsi/sg_lib.h>
#include <scsi/sg_io_linux.h>
#include <signal.h>

#include <string>

#include "lfsr.h"

static void print_usage(const std::string p) {
	fprintf(stderr, "Usage:\n\n%s [rw] [rs] [0-9]+ [0-9]+ [0123] /dev/sg[0-9]+ <logfile>\n", p.c_str());
	exit(-2);
}

static void getout(int s) {
	exit(-1);
}

static std::string strtime() {
	time_t t;
	time(&t);
	struct tm * mytm = gmtime(&t);
	std::string retval(asctime(mytm));
	retval.pop_back();
	return retval;
}

/*
 * Exit codes:
 * 0  : Normal Termination
 * -1 : caught SIGTERM
 * -2 : error in command line parameters
 * -3 : error accessing the device
 */
int main(int argc, char **argv) {
	if (SIG_ERR == signal(SIGINT, getout)) {
		fprintf(stderr, "Cannot manage SIGINT: %s\n", strerror(errno));
		return -1;
	}
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
	const std::string device(argv[6]);
	const std::string logfilename(argv[7]);

	FILE *logfp = fopen(logfilename.c_str(), "w");

	if (!logfp) fprintf(stderr, "%s", strerror(errno));

	if (logfp) fprintf(logfp, "%s UTC: %s %c %c %hu %hu %hu %s %s\n", strtime().c_str(), argv[0], argv[1][0], argv[2][0], iosize, qdepth, key, device.c_str(), logfilename.c_str());

	uint64_t blocks_accessed = 0;

	const int fd = sg_cmds_open_device(device.c_str(), 0, 0);
	if (fd < 0) {
		fprintf(stderr, "Error opening device %s: %s\n", device.c_str(), strerror(errno));
		return -3;
       	}

	uint64_t max_lba = 0;
       	uint32_t blocksize = 0;
       	{
			unsigned char resp[32];
			bzero(resp, 32);
			if (0 != sg_ll_readcap_16(fd, 0, 0, resp, 32, 0, 0)) {
			       	fprintf(stderr, "Read capacity failed on device %s: %s\n", device.c_str(), strerror(errno));
				return -3;
			}
			for (unsigned int i = 0; i < 8; i++) {
				max_lba = (max_lba << 8) + resp[i];
			}
			for (unsigned int i = 8; i < 12; i++) {
				blocksize = (blocksize << 8) + resp[i];
			}
       	}
	if (logfp) fprintf(logfp, "%s UTC: Max LBA %lu Block Size %u\n", strtime().c_str(), max_lba, blocksize);
	sg_cmds_close_device(fd);

	if (logfp) fprintf(logfp, "%s UTC: %lu blocks accessed\n", strtime().c_str(), blocks_accessed);

	if (logfp) fclose(logfp);

	return 0;

	key = key;
	is_write = is_write;
	is_random = is_random;
	print_usage(device);
}
