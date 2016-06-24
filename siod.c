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
#include <unistd.h>
#include <sys/time.h>

#include <string>
#include <iostream>

#include "lfsr.h"

#define CDB_SIZE     ((uint16_t)16)
#define SENSE_LENGTH ((uint16_t)32)
#define STATUS_BLKCNT ((uint64_t)2 * 1024 * 1024)

typedef struct tagIO {
	bool used;
	uint16_t me;
	double start, end;
	unsigned char cdb[CDB_SIZE];
	unsigned char sb[SENSE_LENGTH];
} IO;

static FILE *logfp = NULL;
static uint64_t blocks_accessed = 0;


static std::string buffer2str(const unsigned char * const buf, const uint16_t & sz) {
	std::string retval;
	for (uint16_t i = 0; i < sz; i++) {
		char str[6];
		sprintf(str, "0x%02X ", ((uint8_t)buf[i]));
		retval += str;
	}
	return retval;
}

static std::string cdb2str(const unsigned char * const cdb) {
	return buffer2str(cdb, CDB_SIZE);
}

static std::string sense2str(const unsigned char * const sense) {
	return buffer2str(sense, SENSE_LENGTH);
}

static std::string strtime() {
	time_t t;
	time(&t);
	struct tm * mytm = gmtime(&t);
	std::string retval(asctime(mytm));
	retval.pop_back();
	return retval;
}

static double time2double(const struct timeval & t) {
	return (double)(t.tv_sec) + ((double)t.tv_usec)/(1000 * 1000);
}

static double gettime() {
       	struct timeval t;
       	if (gettimeofday(&t, NULL)) {
		return -1;
	}
       	return time2double(t);
}

static unsigned char *RandomData(const uint8_t & key, const uint64_t & address, const uint16_t & blocksize) {
       	uint64_t x = 0;
       	if (!blocksize) abort();
       	if (blocksize % sizeof(x)) abort();
       	unsigned char * const data = new unsigned char[blocksize];
       	if (NULL == data) {
	       	if (logfp) fprintf(logfp, "%s UTC: Out of memory\n", strtime().c_str());
	       	exit(-4);
	}
	if (key) {
	       	LFSR lfsr(0xFFFFFFFFFFFFFFFFUL, address ? address : address - 1);
	       	for (uint16_t i = 0; i < blocksize; i += sizeof(x)) {
		       	for (uint16_t j = 0; j < (key & 0x3); j++) {
			       	x = lfsr++;
		       	}
		       	unsigned char *ptr = (unsigned char *)(&x);
		       	unsigned char mask = (unsigned char)(key & 0x3) << 6;
		       	for (uint16_t j = 0; j < sizeof(x); j++) {
			       	unsigned char c = *ptr;
			       	c = (c & 0x3F) | mask;
			       	*ptr = c;
			       	ptr++;
		       	}
		       	memcpy(data + i, &x, sizeof(x));
	       	}
       	} else {
	       	memset(data, 0, blocksize);
       	}
       	return data;
}

static bool WrongData(const uint8_t & key, const uint64_t & address, const uint16_t & blocksize, const unsigned char * const data) {
	if (!key) return true;
	const unsigned char * const xdata = RandomData(key, address, blocksize);
	for (uint16_t i = 0; i < blocksize; i++) {
		if (xdata[i] != data[i]) {
			delete [] xdata;
			return true;
		}
	}
	delete [] xdata;
	return false;
}

static void do_io(const uint8_t & key, const int & fd, const uint32_t & blocksize, const uint64_t & offset, const uint16_t & length, IO & io, const char & opcode, const int & dxfer_direction) {
	io.used = true;
	io.start = io.end = gettime();
       	memset(io.cdb, 0, CDB_SIZE);
	io.cdb[0] = opcode;
	io.cdb[1] = 0x08;
       	for (unsigned int j = 0; j < 8; j++) {
	       	io.cdb[2 + j] = (offset >> (8 * (8 - j - 1))) & 0xFF;
       	}
       	for (unsigned int j = 0; j < 2; j++) {
	       	io.cdb[12 + j] = (length >> (8 * (2 - j - 1))) & 0xFF;
       	}
       	sg_io_hdr_t io_hdr;
       	memset(&io_hdr, 0, sizeof(sg_io_hdr_t));
       	io_hdr.interface_id    = 'S';
       	io_hdr.cmd_len         = CDB_SIZE;
       	io_hdr.mx_sb_len       = SENSE_LENGTH;
       	io_hdr.dxfer_direction = dxfer_direction;
       	io_hdr.dxfer_len       = length * blocksize;
	if (SG_DXFER_FROM_DEV == io_hdr.dxfer_direction) {
	       	io_hdr.dxferp = new unsigned char[io_hdr.dxfer_len];
	       	if (NULL == io_hdr.dxferp) {
		       	if (logfp) fprintf(logfp, "%s UTC: Out of memory\n", strtime().c_str());
		       	exit(-4);
	       	}
		memset(io_hdr.dxferp, 0, io_hdr.dxfer_len);
	} else {
	       	io_hdr.dxferp = RandomData(key, offset, blocksize);
	}
       	io_hdr.cmdp            = io.cdb;
       	io_hdr.sbp             = io.sb;
       	io_hdr.timeout         = 20000;     /* 20000 millisecs == 20 seconds */
       	io_hdr.flags           = SG_FLAG_LUN_INHIBIT | SG_FLAG_DIRECT_IO;
	io_hdr.pack_id         = io.me;
	io_hdr.usr_ptr         = &io;
	if (-1 == write(fd, &io_hdr, sizeof(sg_io_hdr_t))) {
		const std::string s(strtime());
	       	if (logfp) fprintf(logfp, "%s UTC: Error : Cannot issue I/O ((write) %s)\n", s.c_str(), strerror(errno));
	       	if (logfp) fprintf(logfp, "%s UTC: Time  : %lf %lf\n", s.c_str(), io.start, io.end);
	       	if (logfp) fprintf(logfp, "%s UTC: CDB   : %s\n", s.c_str(), cdb2str(io.cdb).c_str());
		io.used = false;
       	}
}

static void do_write(const uint8_t & key, const int & fd, const uint32_t & blocksize, const uint64_t & offset, const uint16_t & length, IO & io) {
       	do_io(key, fd, blocksize, offset, length, io, 0x8A, SG_DXFER_TO_DEV);
}

static void do_read(const uint8_t & key, const int & fd, const uint32_t & blocksize, const uint64_t & offset, const uint16_t & length, IO & io) {
       	do_io(key, fd, blocksize, offset, length, io, 0x88, SG_DXFER_FROM_DEV);
}

static void do_wait(const int & fd, const uint16_t & blocksize) {
       	fd_set fds;
       	FD_ZERO(&fds);
	FD_SET(fd, &fds);
	switch (pselect(fd + 1, &fds, NULL, NULL, NULL, NULL)) {
		case -1:
			/* cannot wait on I/O */
		       	if (logfp) fprintf(logfp, "%s UTC: Error : Unable to wait on I/O ((pselect) %s)\n", strtime().c_str(), strerror(errno));
			exit(-5);
		case 0:
			/* severe error, control should never come here */
		       	if (logfp) fprintf(logfp, "%s UTC: Error : Unable to wait on I/O\n", strtime().c_str());
			exit(-5);
		default:
			if (!FD_ISSET(fd, &fds)) {
			       	if (logfp) fprintf(logfp, "%s UTC: Error : Unable to wait on I/O (FD_ISSET)\n", strtime().c_str());
			       	exit(-5);
			}
		       	sg_io_hdr_t io_hdr;
		       	memset(&io_hdr, 0, sizeof(sg_io_hdr_t));
		       	if (-1 == read(fd, &io_hdr, sizeof(sg_io_hdr_t))) {
			       	if (logfp) fprintf(logfp, "%s UTC: Error : Unable to wait on I/O (read)\n", strtime().c_str());
			       	exit(-5);
		       	} 
			IO * const io = (IO *)(io_hdr.usr_ptr);
			if (NULL == io) {
			       	if (logfp) fprintf(logfp, "%s UTC: Error : Unable to wait on I/O (usr_ptr)\n", strtime().c_str());
			       	exit(-5);
		       	} 
			io->end = gettime();
			io->used = false;
			const std::string s(strtime());
			bool error = false;
		       	switch (io_hdr.masked_status) {
				case GOOD:
					break;
			       	case CHECK_CONDITION:
					error = true;
				       	if (logfp) fprintf(logfp, "%s UTC: Error : Check Condition\n", s.c_str());
					break;
			       	case CONDITION_GOOD:
					error = true;
				       	if (logfp) fprintf(logfp, "%s UTC: Error : Condition Good\n", s.c_str());
					break;
			       	case INTERMEDIATE_GOOD:
					error = true;
				       	if (logfp) fprintf(logfp, "%s UTC: Error : Intermediate Good\n", s.c_str());
					break;
			       	case INTERMEDIATE_C_GOOD:
					error = true;
				       	if (logfp) fprintf(logfp, "%s UTC: Error : Intermediate C Good\n", s.c_str());
					break;
			       	case BUSY:
					error = true;
				       	if (logfp) fprintf(logfp, "%s UTC: Error : BUSY\n", s.c_str());
					break;
			       	case RESERVATION_CONFLICT:
					error = true;
				       	if (logfp) fprintf(logfp, "%s UTC: Error : Reservation Conflict\n", s.c_str());
					break;
			       	case COMMAND_TERMINATED:
					error = true;
				       	if (logfp) fprintf(logfp, "%s UTC: Error : Command Terminated\n", s.c_str());
					break;
			       	case QUEUE_FULL:
					error = true;
				       	if (logfp) fprintf(logfp, "%s UTC: Error : Queue Full\n", s.c_str());
					break;
				default:
					error = true;
				       	if (logfp) fprintf(logfp, "%s UTC: Error : Unknown Condition (0x%02X)\n", s.c_str(), io_hdr.masked_status);
					break;
			}
			switch (io_hdr.host_status) {
			       	case SG_LIB_DID_OK: 
					break;
			       	case SG_LIB_DID_NO_CONNECT: 
					error = true;
				       	if (logfp) fprintf(logfp, "%s UTC: Error : HOST: UNABLE TO CONNECT BEFORE TIMEOUT\n", s.c_str());
					break;
			       	case SG_LIB_DID_BUS_BUSY: 
					error = true;
				       	if (logfp) fprintf(logfp, "%s UTC: Error : HOST: BUS BUSY TILL TIMEOUT\n", s.c_str());
					break;
			       	case SG_LIB_DID_TIME_OUT: 
					error = true;
				       	if (logfp) fprintf(logfp, "%s UTC: Error : HOST: TIMEOUT\n", s.c_str());
					break;
			       	case SG_LIB_DID_BAD_TARGET: 
					error = true;
				       	if (logfp) fprintf(logfp, "%s UTC: Error : HOST: BAD TARGET\n", s.c_str());
					break;
			       	case SG_LIB_DID_ABORT: 
					error = true;
				       	if (logfp) fprintf(logfp, "%s UTC: Error : HOST: ABORT\n", s.c_str());
					break;
			       	case SG_LIB_DID_PARITY: 
					error = true;
				       	if (logfp) fprintf(logfp, "%s UTC: Error : HOST: PARITY ERROR ON SCSI BUS\n", s.c_str());
					break;
			       	case SG_LIB_DID_ERROR: 
					error = true;
				       	if (logfp) fprintf(logfp, "%s UTC: Error : HOST: INTERNAL ERROR\n", s.c_str());
					break;
			       	case SG_LIB_DID_RESET: 
					error = true;
				       	if (logfp) fprintf(logfp, "%s UTC: Error : HOST: RESET\n", s.c_str());
					break;
			       	case SG_LIB_DID_BAD_INTR: 
					error = true;
				       	if (logfp) fprintf(logfp, "%s UTC: Error : HOST: RECEIVED AN UNEXPECTED  INTERRUPT\n", s.c_str());
					break;
			       	case SG_LIB_DID_PASSTHROUGH: 
					error = true;
				       	if (logfp) fprintf(logfp, "%s UTC: Error : HOST: FORCE COMMAND PAST MID-LEVEL\n", s.c_str());
					break;
			       	case SG_LIB_DID_SOFT_ERROR: 
					error = true;
				       	if (logfp) fprintf(logfp, "%s UTC: Error : HOST: THE LOW LEVEL DRIVER WANTS A RETRY\n", s.c_str());
					break;
	       			default:
					error = true;
				       	if (logfp) fprintf(logfp, "%s UTC: Error : UNKNOWN HOST STATUS (0x%02X)\n", s.c_str(), io_hdr.host_status);
					break;
			}
			switch (io_hdr.driver_status & 0xF) {
			       	case SG_LIB_DRIVER_OK: 
					break;
				case SG_LIB_DRIVER_BUSY: 
					error = true;
				       	if (logfp) fprintf(logfp, "%s UTC: Error : DRIVER BUSY\n", s.c_str());
					break;
				case SG_LIB_DRIVER_SOFT: 
					error = true;
				       	if (logfp) fprintf(logfp, "%s UTC: Error : DRIVER SOFTWARE ERROR\n", s.c_str());
					break;
				case SG_LIB_DRIVER_MEDIA: 
					error = true;
				       	if (logfp) fprintf(logfp, "%s UTC: Error : DRIVER MEDIA ERROR\n", s.c_str());
					break;
				case SG_LIB_DRIVER_ERROR: 
					error = true;
				       	if (logfp) fprintf(logfp, "%s UTC: Error : DRIVER ERROR\n", s.c_str());
					break;
				case SG_LIB_DRIVER_INVALID: 
					error = true;
				       	if (logfp) fprintf(logfp, "%s UTC: Error : DRIVER INVALID ERROR\n", s.c_str());
					break;
				case SG_LIB_DRIVER_TIMEOUT: 
					error = true;
				       	if (logfp) fprintf(logfp, "%s UTC: Error : DRIVER TIMEOUT ERROR\n", s.c_str());
					break;
				case SG_LIB_DRIVER_HARD: 
					error = true;
				       	if (logfp) fprintf(logfp, "%s UTC: Error : DRIVER HARDWARE ERROR\n", s.c_str());
					break;
				case SG_LIB_DRIVER_SENSE: 
					error = true;
				       	if (logfp) fprintf(logfp, "%s UTC: Error : DRIVER SENSE ERROR\n", s.c_str());
					break;
				default:
					error = true;
				       	if (logfp) fprintf(logfp, "%s UTC: Error : UNKNOWN DRIVER STATUS (0x%02X)\n", s.c_str(), io_hdr.driver_status);
					break;
			}
			if (error) {
			       	if (logfp) fprintf(logfp, "%s UTC: Time  : %lf %lf\n", s.c_str(), io->start, io->end);
			       	if (logfp) fprintf(logfp, "%s UTC: CDB   : %s\n", s.c_str(), cdb2str(io->cdb).c_str());
			       	if (logfp) fprintf(logfp, "%s UTC: Sense : %s\n", s.c_str(), sense2str(io->sb).c_str());
			}
		       	if (SG_DXFER_FROM_DEV == io_hdr.dxfer_direction) {
				const uint8_t key = (((unsigned char *)io_hdr.dxferp)[0] >> 6) & 0x3;
				uint64_t address = 0;
			       	for (unsigned int j = 0; j < 8; j++) {
				       	address = (address << 8) | io_hdr.cmdp[2 + j];
			       	}
			       	if (WrongData(key, address, blocksize, (unsigned char *)io_hdr.dxferp)) {
				       	if (logfp) fprintf(logfp, "%s UTC: Error : Data Miscompare\n", s.c_str());
				       	if (logfp) fprintf(logfp, "%s UTC: Time  : %lf %lf\n", s.c_str(), io->start, io->end);
				       	if (logfp) fprintf(logfp, "%s UTC: CDB   : %s\n", s.c_str(), cdb2str(io->cdb).c_str());
				}
			}
		       	delete [] ((unsigned char *)io_hdr.dxferp);
			break;
	}
}

static void print_usage(const std::string p) {
	fprintf(logfp, "Usage:\n\n%s [rw] [rs] [0-9]+ [0-9]+ [0123] /dev/sg[0-9]+ <logfile>\n", p.c_str());
	exit(-2);
}

static void getout(int s) {
	if (logfp) fprintf(logfp, "%s UTC: Caught signal (%d) %s\n", strtime().c_str(), s, strsignal(s));
	if (logfp) fprintf(logfp, "%s UTC: %lu blocks accessed\n", strtime().c_str(), blocks_accessed);
	if (logfp) fclose(logfp);
	exit(-1);
}

/*
 * Exit codes:
 * 0  : Normal Termination
 * -1 : caught signal (SIGINT, SIGTERM)
 * -2 : error in command line parameters
 * -3 : error accessing the device
 * -4 : out of memory
 * -5 : cannot wait on I/Os, problem with pselect system call
 */
int main(int argc, char **argv) {

	if (!logfp) logfp = stderr;

	if (SIG_ERR == signal(SIGHUP, SIG_IGN)) {
		fprintf(logfp, "Cannot manage SIGHUP: %s\n", strerror(errno));
		return -1;
	}
	if (SIG_ERR == signal(SIGINT, getout)) {
		fprintf(logfp, "Cannot manage SIGINT: %s\n", strerror(errno));
		return -1;
	}
	if (SIG_ERR == signal(SIGQUIT, getout)) {
		fprintf(logfp, "Cannot manage SIGQUIT: %s\n", strerror(errno));
		return -1;
	}
	if (SIG_ERR == signal(SIGILL, getout)) {
		fprintf(logfp, "Cannot manage SIGILL: %s\n", strerror(errno));
		return -1;
	}
	if (SIG_ERR == signal(SIGABRT, getout)) {
		fprintf(logfp, "Cannot manage SIGABRT: %s\n", strerror(errno));
		return -1;
	}
	if (SIG_ERR == signal(SIGFPE, getout)) {
		fprintf(logfp, "Cannot manage SIGFPE: %s\n", strerror(errno));
		return -1;
	}
	if (SIG_ERR == signal(SIGSEGV, getout)) {
		fprintf(logfp, "Cannot manage SIGSEGV: %s\n", strerror(errno));
		return -1;
	}
	if (SIG_ERR == signal(SIGPIPE, getout)) {
		fprintf(logfp, "Cannot manage SIGPIPE: %s\n", strerror(errno));
		return -1;
	}
	if (SIG_ERR == signal(SIGALRM, getout)) {
		fprintf(logfp, "Cannot manage SIGALRM: %s\n", strerror(errno));
		return -1;
	}
	if (SIG_ERR == signal(SIGTERM, getout)) {
		fprintf(logfp, "Cannot manage SIGTERM: %s\n", strerror(errno));
		return -1;
	}
	if (SIG_ERR == signal(SIGUSR1, SIG_IGN)) {
		fprintf(logfp, "Cannot manage SIGUSR1: %s\n", strerror(errno));
		return -1;
	}
	if (SIG_ERR == signal(SIGUSR2, SIG_IGN)) {
		fprintf(logfp, "Cannot manage SIGUSR2: %s\n", strerror(errno));
		return -1;
	}
	if (SIG_ERR == signal(SIGCHLD, SIG_IGN)) {
		fprintf(logfp, "Cannot manage SIGCHLD: %s\n", strerror(errno));
		return -1;
	}
	if (SIG_ERR == signal(SIGTTIN, SIG_IGN)) {
		fprintf(logfp, "Cannot manage SIGTTIN: %s\n", strerror(errno));
		return -1;
	}
	if (SIG_ERR == signal(SIGTTOU, SIG_IGN)) {
		fprintf(logfp, "Cannot manage SIGTTOU: %s\n", strerror(errno));
		return -1;
	}
	if (SIG_ERR == signal(SIGBUS, getout)) {
		fprintf(logfp, "Cannot manage SIGBUS: %s\n", strerror(errno));
		return -1;
	}
	if (SIG_ERR == signal(SIGPOLL, getout)) {
		fprintf(logfp, "Cannot manage SIGPOLL: %s\n", strerror(errno));
		return -1;
	}
	if (SIG_ERR == signal(SIGPROF, SIG_IGN)) {
		fprintf(logfp, "Cannot manage SIGPROF: %s\n", strerror(errno));
		return -1;
	}
	if (SIG_ERR == signal(SIGSYS, getout)) {
		fprintf(logfp, "Cannot manage SIGSYS: %s\n", strerror(errno));
		return -1;
	}
	if (SIG_ERR == signal(SIGTRAP, getout)) {
		fprintf(logfp, "Cannot manage SIGTRAP: %s\n", strerror(errno));
		return -1;
	}
	if (SIG_ERR == signal(SIGURG, SIG_IGN)) {
		fprintf(logfp, "Cannot manage SIGURG: %s\n", strerror(errno));
		return -1;
	}
	if (SIG_ERR == signal(SIGVTALRM, getout)) {
		fprintf(logfp, "Cannot manage SIGVTALRM: %s\n", strerror(errno));
		return -1;
	}
	if (SIG_ERR == signal(SIGXCPU, getout)) {
		fprintf(logfp, "Cannot manage SIGXCPU: %s\n", strerror(errno));
		return -1;
	}
	if (SIG_ERR == signal(SIGXFSZ, getout)) {
		fprintf(logfp, "Cannot manage SIGXFSZ: %s\n", strerror(errno));
		return -1;
	}
	/* NOT DEFINED
	if (SIG_ERR == signal(SIGEMT, getout)) {
		fprintf(logfp, "Cannot manage SIGEMT: %s\n", strerror(errno));
		return -1;
	}
	*/
	if (SIG_ERR == signal(SIGSTKFLT, getout)) {
		fprintf(logfp, "Cannot manage SIGSTKFLT: %s\n", strerror(errno));
		return -1;
	}
	if (SIG_ERR == signal(SIGIO, SIG_IGN)) {
		fprintf(logfp, "Cannot manage SIGIO: %s\n", strerror(errno));
		return -1;
	}
	if (SIG_ERR == signal(SIGPWR, getout)) {
		fprintf(logfp, "Cannot manage SIGPWR: %s\n", strerror(errno));
		return -1;
	} 
	/* NOT DEFINED
	if (SIG_ERR == signal(SIGLOST, SIG_IGN)) {
		fprintf(logfp, "Cannot manage SIGLOST: %s\n", strerror(errno));
		return -1;
	}
	*/
	if (SIG_ERR == signal(SIGWINCH, SIG_IGN)) {
		fprintf(logfp, "Cannot manage SIGWINCH: %s\n", strerror(errno));
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
	IO *ios = new IO[qdepth];
	if (NULL == ios) {
		fprintf(logfp, "Out of memory.\n");
		return -4;
	}
	for (uint16_t i = 0; i < qdepth; i++) {
	       	memset(&ios[i], 0, sizeof(IO));
		ios[i].used = false;
		ios[i].me = i;
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

	logfp = fopen(logfilename.c_str(), "w");
	if (!logfp) fprintf(stderr, "Unable to open logfile %s: %s", logfilename.c_str(), strerror(errno));
	if (!logfp) logfp = stderr;


	if (logfp) fprintf(logfp, "%s UTC: %s %c %c %hu %hu %hu %s %s\n", strtime().c_str(), argv[0], argv[1][0], argv[2][0], iosize, qdepth, key, device.c_str(), logfilename.c_str());

	const int fd = sg_cmds_open_device(device.c_str(), 0, 0);
	if (fd < 0) {
		fprintf(logfp, "Error opening device %s: %s\n", device.c_str(), strerror(errno));
		return -3;
       	}

	uint64_t max_lba = 0;
       	uint32_t blocksize = 0;
       	{
			unsigned char resp[32];
			bzero(resp, 32);
			if (0 != sg_ll_readcap_16(fd, 0, 0, resp, 32, 0, 0)) {
			       	fprintf(logfp, "Read capacity failed on device %s: %s\n", device.c_str(), strerror(errno));
				return -3;
			}
			for (unsigned int i = 0; i < 8; i++) {
				max_lba = (max_lba << 8) + resp[i];
			}
			for (unsigned int i = 8; i < 12; i++) {
				blocksize = (blocksize << 8) + resp[i];
			}
       	}
	if (logfp) fprintf(logfp, "%s UTC: Max LBA %8lX Block Size %u\n", strtime().c_str(), max_lba, blocksize);

	const uint64_t max_offsets = (max_lba + 1) / iosize + (((max_lba + 1) % iosize) ? 1 : 0);
	Offset *offset = NULL;
	if (is_random) offset = new RandomOffset(max_offsets);
	else           offset = new SequentialOffset(max_offsets);
	if (NULL == offset) {
		fprintf(logfp, "Out of memory.\n");
		return -4;
	}
	const uint64_t last = *offset;
	uint64_t next_status_print = STATUS_BLKCNT;
	do {
		const uint64_t a = *offset;
		const uint64_t b =  a * iosize;
		const uint32_t c =  (b + iosize - 1 <= max_lba) ? iosize : max_lba - b + 1;
	       	for (uint16_t i = 0; i < qdepth; i++) {
			if (false == ios[i].used) {
				if (is_write) {
					do_write(key, fd, blocksize, b, c, ios[i]);
				} else {
					do_read( key, fd, blocksize, b, c, ios[i]);
				}
				blocks_accessed += c;
			}
		}
		do_wait(fd, blocksize);
		if (blocks_accessed >= next_status_print) {
		       	if (logfp) fprintf(logfp, "%s UTC: %8lX blocks accessed (%6.2lf%% done)\n", strtime().c_str(), blocks_accessed, double(blocks_accessed * 100) / double(max_lba + 1));
			if (logfp) fflush(logfp);
		       	next_status_print += STATUS_BLKCNT;
		}
	} while (last != offset->Next());

	delete offset;
	offset = NULL;
	delete [] ios;
	ios = NULL;

	sg_cmds_close_device(fd);

	if (logfp) fprintf(logfp, "%s UTC: %lu blocks accessed\n", strtime().c_str(), blocks_accessed);

	if (logfp) fclose(logfp);

	return 0;
}
