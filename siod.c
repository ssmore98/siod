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
#include <sys/wait.h>
#include <zlib.h>

#include <string>
#include <iostream>
#include <vector>
#include <set>
#include <iomanip>
#include <sstream>

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
	int fd;
} IO;

static gzFile   logfp           = 0;
static uint64_t blocks_accessed = 0;
static bool     data_miscompare = false;

typedef enum {ErrorNone, ErrorSyscall, ErrorIO, ErrorData, ErrorSignal, ErrorInternal} ErrorType;

static std::string strtime() {
	time_t t;
	if (0 > time(&t)) {
		return std::string();
	}
	struct tm * mytm = gmtime(&t);
	if (NULL == mytm) {
		return std::string();
	}
	const char * const ts = asctime(mytm);
	if (NULL == ts) {
		return std::string();
	}
	std::string retval(ts);
	retval.pop_back();
	return retval;
}

static std::string cdb2str(const unsigned char * const cdb);
static std::string sense2str(const unsigned char * const sense);

static void makeexit(const int & status) {
	if (logfp) gzprintf(logfp,  "%s UTC: 0x%08lX blocks accessed\n", strtime().c_str(), blocks_accessed);
	else        fprintf(stderr, "%s UTC: 0x%08lX blocks accessed\n", strtime().c_str(), blocks_accessed);
       	if (logfp) gzclose(logfp);
	exit(status);
}

static void myprintf(const char * const file, const int & line, const ErrorType & e, const bool & fatal, const std::string & s = std::string(), const double & start = 0, const double & end = 0,
	       	const unsigned char * const cdb = NULL, const unsigned char * const sbp = NULL) {
	std::ostringstream head;
	head << strtime() << " UTC (" << file << ':' << line << ": ";
	switch (e) {
		case ErrorSyscall:
			{
			       	std::string errmsg(strerror(errno));
			       	if (logfp) gzprintf(logfp,  "%s Syscall error : %s (%s)\n", head.str().c_str(), s.c_str(), errmsg.c_str());
			       	else        fprintf(stderr, "%s Syscall error : %s (%s)\n", head.str().c_str(), s.c_str(), errmsg.c_str());
			}
			if (fatal) makeexit(-4);
			break;
		case ErrorInternal:
		       	if (logfp) gzprintf(logfp, "%s Internal error : %s\n", head.str().c_str(), s.c_str());
		       	else        fprintf(stderr, "%s Internal error : %s\n", head.str().c_str(), s.c_str());
			if (fatal) makeexit(-8);
			break;
		case ErrorSignal:
		       	if (logfp) gzprintf(logfp, "%s Signal error : %s\n", head.str().c_str(), s.c_str());
		       	else        fprintf(stderr, "%s Signal error : %s\n", head.str().c_str(), s.c_str());
			if (fatal) makeexit(-1);
			break;
		case ErrorIO:
			{
			       	std::string errmsg(strerror(errno));
			       	if (logfp) gzprintf(logfp,  "%s IO error : %s (%s) : %lf %lf : %s : %s\n", head.str().c_str(), s.c_str(), errmsg.c_str(), start, end, cdb2str(cdb).c_str(), sense2str(sbp).c_str());
			       	else        fprintf(stderr, "%s IO error : %s (%s) : %lf %lf : %s : %s\n", head.str().c_str(), s.c_str(), errmsg.c_str(), start, end, cdb2str(cdb).c_str(), sense2str(sbp).c_str());
			}
			break;
		case ErrorData:
		       	if (logfp) gzprintf(logfp,  "%s Data error : %lf %lf : %s\n", head.str().c_str(), start, end, cdb2str(cdb).c_str());
		       	else        fprintf(stderr, "%s Data error : %lf %lf : %s\n", head.str().c_str(), start, end, cdb2str(cdb).c_str());
			break;
		default:
		       	if (logfp) gzprintf(logfp, "%s Unknown error\n", head.str().c_str());
		       	else        fprintf(stderr, "%s Unknown error\n", head.str().c_str());
			break;
	}
}

static std::string buffer2str(const unsigned char * const buf, const uint16_t & sz) {
	if (!buf || !sz) return std::string();
	std::string retval;
	for (uint16_t i = 0; i < sz; i++) {
		char str[6];
		if (0 > snprintf(str, 6, "0x%02X ", ((uint8_t)buf[i]))) {
			myprintf(__FILE__, __LINE__, ErrorSyscall, true, "snprintf");
		}
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

static unsigned char *RandomData(const uint8_t & key, const uint64_t & address, const uint16_t & blocksize, const uint16_t & blocks) {
       	if (!blocks) {
		myprintf(__FILE__, __LINE__, ErrorInternal, true, std::string("Buffer size error ") + std::to_string(blocks));
	}
       	uint64_t x = 0;
       	if ((!blocksize) || (blocksize % sizeof(x))) {
		myprintf(__FILE__, __LINE__, ErrorInternal, true, std::string("Block size error ") + std::to_string(blocksize));
	}
       	unsigned char * const data = new unsigned char[blocksize * blocks];
       	if (NULL == data) {
	       	myprintf(__FILE__, __LINE__, ErrorSyscall, true, "malloc");
	}
	if (key) {
	       	const uint64_t mask = (uint64_t)(key & 0x3) << 62;
		for (uint16_t k = 0; k < blocks; k++) {
		       	LFSR lfsr(0x3FFFFFFFFFFFFFFFUL, (address + k) ? (address + k) : (address + k - 1));
		       	for (uint16_t j = 0; j < (key & 0x3); j++) {
			       	x = lfsr++;
		       	}
			x = (x & 0x3FFFFFFFFFFFFFFFUL) | mask;
		       	for (uint16_t i = 0; i < blocksize; i += sizeof(x)) {
			       	if (data + k * blocksize + i != memcpy(data + k * blocksize + i, &x, sizeof(x))) {
				       	myprintf(__FILE__, __LINE__, ErrorSyscall, true, "memcpy");
			       	}
		       	}
		}
       	} else {
	       	if (data != memset(data, 0, blocksize * blocks)) {
		       	myprintf(__FILE__, __LINE__, ErrorSyscall, true, "memset");
		       	if (logfp) gzclose(logfp);
		       	exit(-4);
		}
       	}
       	return data;
}

static bool WrongData(const uint8_t & key, const uint64_t & address, const uint16_t & blocksize, const uint16_t & blocks, const unsigned char * const data) {
	if (!key) return true;
	const unsigned char * const xdata = RandomData(key, address, blocksize, blocks);
	for (uint16_t i = 0; i < blocksize; i++) {
		if (xdata[i] != data[i]) {
			delete [] xdata;
			return true;
		}
	}
	delete [] xdata;
	return false;
}

static void do_io(const uint8_t & key, const uint32_t & blocksize, const uint64_t & offset, const uint16_t & length, IO & io, const char & opcode, const int & dxfer_direction) {
	io.used = true;
	io.start = io.end = gettime();
       	if (io.cdb != memset(io.cdb, 0, CDB_SIZE)) {
	       	myprintf(__FILE__, __LINE__, ErrorSyscall, true, "memset");
	}
	io.cdb[0] = opcode;
	io.cdb[1] = 0x08;
       	for (unsigned int j = 0; j < 8; j++) {
	       	io.cdb[2 + j] = (offset >> (8 * (8 - j - 1))) & 0xFF;
       	}
       	for (unsigned int j = 0; j < 2; j++) {
	       	io.cdb[12 + j] = (length >> (8 * (2 - j - 1))) & 0xFF;
       	}
       	sg_io_hdr_t io_hdr;
       	if (&io_hdr != memset(&io_hdr, 0, sizeof(sg_io_hdr_t))) {
	       	myprintf(__FILE__, __LINE__, ErrorSyscall, true, "memset");
	}
       	io_hdr.interface_id    = 'S';
       	io_hdr.cmd_len         = CDB_SIZE;
       	io_hdr.mx_sb_len       = SENSE_LENGTH;
       	io_hdr.dxfer_direction = dxfer_direction;
       	io_hdr.dxfer_len       = length * blocksize;
	if (SG_DXFER_FROM_DEV == io_hdr.dxfer_direction) {
	       	io_hdr.dxferp = new unsigned char[io_hdr.dxfer_len];
	       	if (NULL == io_hdr.dxferp) {
		       	myprintf(__FILE__, __LINE__, ErrorSyscall, true, "malloc");
	       	}
		if (io_hdr.dxferp != memset(io_hdr.dxferp, 0, io_hdr.dxfer_len)) {
		       	myprintf(__FILE__, __LINE__, ErrorSyscall, true, "memset");
		}
	} else {
	       	io_hdr.dxferp = RandomData(key, offset, blocksize, length);
	}
       	io_hdr.cmdp            = io.cdb;
       	io_hdr.sbp             = io.sb;
       	io_hdr.timeout         = 20000;     /* 20000 millisecs == 20 seconds */
       	io_hdr.flags           = SG_FLAG_LUN_INHIBIT | SG_FLAG_DIRECT_IO;
	io_hdr.pack_id         = io.me;
	io_hdr.usr_ptr         = &io;
	if (-1 == write(io.fd, &io_hdr, sizeof(sg_io_hdr_t))) {
		myprintf(__FILE__, __LINE__, ErrorIO, false, "write", io.start, io.end, io.cdb);
		delete [] (unsigned char *)io_hdr.dxferp;
		io.used = false;
       	}
}

static void do_write(const uint8_t & key, const uint32_t & blocksize, const uint64_t & offset, const uint16_t & length, IO & io) {
       	do_io(key, blocksize, offset, length, io, 0x8A, SG_DXFER_TO_DEV);
}

static void do_read(const uint8_t & key, const uint32_t & blocksize, const uint64_t & offset, const uint16_t & length, IO & io) {
       	do_io(key, blocksize, offset, length, io, 0x88, SG_DXFER_FROM_DEV);
}

static void do_wait(const uint8_t & key, const std::set<int> fds, const uint16_t & blocksize) {
       	fd_set FDS;
       	FD_ZERO(&FDS);
	int maxfd = -1;
	for (std::set<int>::const_iterator i = fds.begin(); i != fds.end(); i++) {
	       	FD_SET(*i, &FDS);
		if (*i > maxfd) maxfd = *i;
	}
	maxfd += 1;
	switch (pselect(maxfd, &FDS, NULL, NULL, NULL, NULL)) {
		case -1:
			/* cannot wait on I/O */
		       	myprintf(__FILE__, __LINE__, ErrorSyscall, true, "pselect");
			break;
		case 0:
			/* severe error, control should never come here */
		       	myprintf(__FILE__, __LINE__, ErrorSyscall, true, "pselect");
			break;
		default:
		       	for (std::set<int>::const_iterator fd = fds.begin(); fd != fds.end(); fd++) {
			       	if (FD_ISSET(*fd, &FDS)) {
				       	sg_io_hdr_t io_hdr;
				       	if (&io_hdr != memset(&io_hdr, 0, sizeof(sg_io_hdr_t))) {
					       	myprintf(__FILE__, __LINE__, ErrorSyscall, true, "memset");
				       	}
				       	if (-1 == read(*fd, &io_hdr, sizeof(sg_io_hdr_t))) {
					       	myprintf(__FILE__, __LINE__, ErrorSyscall, true, "read");
				       	} 
					IO * const io = (IO *)(io_hdr.usr_ptr);
				       	if (NULL == io) {
					       	myprintf(__FILE__, __LINE__, ErrorSyscall, true, "usr_ptr");
				       	} 
					io->end = gettime();
				       	io->used = false;
				       	const std::string s(strtime());
					const bool in_error = (io_hdr.masked_status == GOOD) && (io_hdr.host_status == SG_LIB_DID_OK) && (io_hdr.driver_status == SG_LIB_DRIVER_OK);
				       	switch (io_hdr.masked_status) {
					       	case GOOD:
						       	break;
					       	case CHECK_CONDITION:
						       	myprintf(__FILE__, __LINE__, ErrorIO, false, "Check Condition", io->start, io->end, io->cdb, io->sb);
						       	break;
					       	case CONDITION_GOOD:
						       	myprintf(__FILE__, __LINE__, ErrorIO, false, "Condition Good", io->start, io->end, io->cdb, io->sb);
							break;
					       	case INTERMEDIATE_GOOD:
						       	myprintf(__FILE__, __LINE__, ErrorIO, false, "Intermediate Good", io->start, io->end, io->cdb, io->sb);
							break;
					       	case INTERMEDIATE_C_GOOD:
						       	myprintf(__FILE__, __LINE__, ErrorIO, false, "Intermediate C Good", io->start, io->end, io->cdb, io->sb);
							break;
					       	case BUSY:
						       	myprintf(__FILE__, __LINE__, ErrorIO, false, "Busy", io->start, io->end, io->cdb, io->sb);
							break;
					       	case RESERVATION_CONFLICT:
						       	myprintf(__FILE__, __LINE__, ErrorIO, false, "Reservation Conflict", io->start, io->end, io->cdb, io->sb);
							break;
					       	case COMMAND_TERMINATED:
						       	myprintf(__FILE__, __LINE__, ErrorIO, false, "Command Terminated", io->start, io->end, io->cdb, io->sb);
							break;
					       	case QUEUE_FULL:
						       	myprintf(__FILE__, __LINE__, ErrorIO, false, "Queue Full", io->start, io->end, io->cdb, io->sb);
							break;
						default:
							{
								std::ostringstream s;
								s << "Unknown Condition (0x" << std::setfill('0') << std::hex << std::setw(2) << io_hdr.masked_status;
							       	myprintf(__FILE__, __LINE__, ErrorIO, false, s.str(), io->start, io->end, io->cdb, io->sb);
							}
							break;
					}
					switch (io_hdr.host_status) {
					       	case SG_LIB_DID_OK: 
							break;
					       	case SG_LIB_DID_NO_CONNECT: 
						       	myprintf(__FILE__, __LINE__, ErrorIO, false, "HOST UNABLE TO CONNECT BEFORE TIMEOUT", io->start, io->end, io->cdb, io->sb);
							break;
					       	case SG_LIB_DID_BUS_BUSY: 
						       	myprintf(__FILE__, __LINE__, ErrorIO, false, "HOST BUS BUSY TILL TIMEOUT", io->start, io->end, io->cdb, io->sb);
							break;
					       	case SG_LIB_DID_TIME_OUT: 
						       	myprintf(__FILE__, __LINE__, ErrorIO, false, "HOST TIMEOUT", io->start, io->end, io->cdb, io->sb);
							break;
					       	case SG_LIB_DID_BAD_TARGET: 
						       	myprintf(__FILE__, __LINE__, ErrorIO, false, "HOST BAD TARGET", io->start, io->end, io->cdb, io->sb);
							break;
					       	case SG_LIB_DID_ABORT: 
						       	myprintf(__FILE__, __LINE__, ErrorIO, false, "HOST ABORT", io->start, io->end, io->cdb, io->sb);
							break;
					       	case SG_LIB_DID_PARITY: 
						       	myprintf(__FILE__, __LINE__, ErrorIO, false, "HOST PARITY ERROR ON SCSI BUS", io->start, io->end, io->cdb, io->sb);
							break;
					       	case SG_LIB_DID_ERROR: 
						       	myprintf(__FILE__, __LINE__, ErrorIO, false, "HOST INTERNAL ERROR", io->start, io->end, io->cdb, io->sb);
							break;
					       	case SG_LIB_DID_RESET: 
						       	myprintf(__FILE__, __LINE__, ErrorIO, false, "HOST RESET", io->start, io->end, io->cdb, io->sb);
							break;
					       	case SG_LIB_DID_BAD_INTR: 
						       	myprintf(__FILE__, __LINE__, ErrorIO, false, "HOST RECEIVED AN UNEXPECTED  INTERRUPT", io->start, io->end, io->cdb, io->sb);
							break;
					       	case SG_LIB_DID_PASSTHROUGH: 
						       	myprintf(__FILE__, __LINE__, ErrorIO, false, "HOST FORCE COMMAND PAST MID-LEVEL", io->start, io->end, io->cdb, io->sb);
							break;
					       	case SG_LIB_DID_SOFT_ERROR: 
						       	myprintf(__FILE__, __LINE__, ErrorIO, false, "HOST THE LOW LEVEL DRIVER WANTS A RETRY", io->start, io->end, io->cdb, io->sb);
							break;
			       			default:
							{
								std::ostringstream s;
								s << "HOST UNKNOWN STATUS (0x" << std::setfill('0') << std::hex << std::setw(2) << io_hdr.host_status;
							       	myprintf(__FILE__, __LINE__, ErrorIO, false, s.str(), io->start, io->end, io->cdb, io->sb);
							}
							break;
					}
					switch (io_hdr.driver_status & 0xF) {
					       	case SG_LIB_DRIVER_OK: 
							break;
						case SG_LIB_DRIVER_BUSY: 
						       	myprintf(__FILE__, __LINE__, ErrorIO, false, "DRIVER BUSY", io->start, io->end, io->cdb, io->sb);
							break;
						case SG_LIB_DRIVER_SOFT: 
						       	myprintf(__FILE__, __LINE__, ErrorIO, false, "DRIVER SOFTWARE ERROR", io->start, io->end, io->cdb, io->sb);
							break;
						case SG_LIB_DRIVER_MEDIA: 
						       	myprintf(__FILE__, __LINE__, ErrorIO, false, "DRIVER MEDIA ERROR", io->start, io->end, io->cdb, io->sb);
							break;
						case SG_LIB_DRIVER_ERROR: 
						       	myprintf(__FILE__, __LINE__, ErrorIO, false, "DRIVER ERROR", io->start, io->end, io->cdb, io->sb);
							break;
						case SG_LIB_DRIVER_INVALID: 
						       	myprintf(__FILE__, __LINE__, ErrorIO, false, "DRIVER INVALID ERROR", io->start, io->end, io->cdb, io->sb);
							break;
						case SG_LIB_DRIVER_TIMEOUT: 
						       	myprintf(__FILE__, __LINE__, ErrorIO, false, "DRIVER TIMEOUT ERROR", io->start, io->end, io->cdb, io->sb);
							break;
						case SG_LIB_DRIVER_HARD: 
						       	myprintf(__FILE__, __LINE__, ErrorIO, false, "DRIVER HARDWARE ERROR", io->start, io->end, io->cdb, io->sb);
							break;
						case SG_LIB_DRIVER_SENSE: 
						       	myprintf(__FILE__, __LINE__, ErrorIO, false, "DRIVER SENSE ERROR", io->start, io->end, io->cdb, io->sb);
							break;
						default:
							{
								std::ostringstream s;
								s << "DRIVER UNKNOWN STATUS (0x" << std::setfill('0') << std::hex << std::setw(2) << io_hdr.driver_status;
							       	myprintf(__FILE__, __LINE__, ErrorIO, false, s.str(), io->start, io->end, io->cdb, io->sb);
							}
							break;
					}
				       	if ((!in_error) && (SG_DXFER_FROM_DEV == io_hdr.dxfer_direction)) {
						uint8_t new_key = key;
						if (!new_key) new_key = (((unsigned char *)io_hdr.dxferp)[0] >> 6) & 0x3;
						uint64_t address = 0;
					       	for (unsigned int j = 0; j < 8; j++) {
						       	address = (address << 8) | io_hdr.cmdp[2 + j];
					       	}
						uint16_t length = 0;
					       	for (unsigned int j = 0; j < 2; j++) {
						       	length = (length << 8) | io_hdr.cmdp[12 + j];
					       	}
					       	if (WrongData(new_key, address, blocksize, length, (unsigned char *)io_hdr.dxferp)) {
						       	myprintf(__FILE__, __LINE__, ErrorData, false, "", io->start, io->end, io->cdb);
							data_miscompare = true;
						}
					}
				       	delete [] ((unsigned char *)io_hdr.dxferp);
				}
			}
			break;
	}
}

static void print_usage(const std::string p) {
	fprintf(stderr, "\nUsage:\n\n%s [rw] [rs] [0-9]+ [0-9]+ [0123] <logfile prefix> [0-9]+ ...\n\n", p.c_str());
       	if (logfp) gzclose(logfp);
	exit(-2);
}

static void getout(int s) {
       	myprintf(__FILE__, __LINE__, ErrorSignal, true, strsignal(s));
	if (logfp) gzclose(logfp);
	exit(-1);
}

static int slave(const std::string & argv0, const bool & is_write, const bool & is_random, const uint16_t & iosize, uint16_t & qdepth,
	       	const uint8_t & key, const std::string & logfile_prefix, const uint16_t & dno) {

	const std::string device = std::string("/dev/sg") + std::to_string(dno);

	// the slave process
	{
		const std::string logfilename = logfile_prefix + std::string("log.") + std::to_string(dno) + std::string(".gz");
	       	logfp = gzopen(logfilename.c_str(), "wb");
	       	if (!logfp) myprintf(__FILE__, __LINE__, ErrorSyscall, false, "Cannot open log file");
	       	if (logfp) gzbuffer(logfp, 256 * 1024);
	       	if (logfp) gzprintf(logfp, "%s UTC: %s %c %c %hu %hu %hu %s %s\n", strtime().c_str(), argv0.c_str(), is_write ? 'W' : 'R', is_random ? 'R' : 'S',
			       	iosize, qdepth, key, device.c_str(), logfilename.c_str());
		else        fprintf(stderr, "%s UTC: %s %c %c %hu %hu %hu %s %s\n", strtime().c_str(), argv0.c_str(), is_write ? 'W' : 'R', is_random ? 'R' : 'S',
			       	iosize, qdepth, key, device.c_str(), logfilename.c_str());
	}

	IO *ios = new IO[qdepth];
	if (NULL == ios) {
	       	myprintf(__FILE__, __LINE__, ErrorSyscall, true, "malloc");
	}
	std::set<int> fds;
	{
	       	int fd = -1;
	       	for (uint16_t i = 0; i < qdepth; i++) {
		       	if (0 == i % SG_MAX_QUEUE) {
			       	fd = sg_cmds_open_device(device.c_str(), 0, 0);
			       	if (fd < 0) {
				       	myprintf(__FILE__, __LINE__, ErrorSyscall, true, "open");
			       	}
				if (false == fds.insert(fd).second) {
				       	myprintf(__FILE__, __LINE__, ErrorInternal, true, "set insert");
				}
		       	}
		       	if (&ios[i] != memset(&ios[i], 0, sizeof(IO))) {
				myprintf(__FILE__, __LINE__, ErrorSyscall, true, "memset");
		       	}
		       	ios[i].used = false;
		       	ios[i].me = i;
		       	ios[i].fd = fd;
	       	}
	}

	uint64_t max_lba = 0;
       	uint32_t blocksize = 0;
       	{
			unsigned char resp[32];
		       	if (resp != memset(resp, 0, sizeof(resp))) {
				myprintf(__FILE__, __LINE__, ErrorSyscall, true, "memset");
		       	}
			if (0 != sg_ll_readcap_16(ios[0].fd, 0, 0, resp, 32, 0, 0)) {
				myprintf(__FILE__, __LINE__, ErrorSyscall, true, "sg_ll_readcap_16");
			}
			for (unsigned int i = 0; i < 8; i++) {
				max_lba = (max_lba << 8) + resp[i];
			}
			for (unsigned int i = 8; i < 12; i++) {
				blocksize = (blocksize << 8) + resp[i];
			}
       	}
	if (logfp) gzprintf(logfp,  "%s UTC: Max LBA %8lX Block Size %u\n", strtime().c_str(), max_lba, blocksize);
	else        fprintf(stderr, "%s UTC: Max LBA %8lX Block Size %u\n", strtime().c_str(), max_lba, blocksize);

	const uint64_t max_offsets = (max_lba + 1) / iosize + (((max_lba + 1) % iosize) ? 1 : 0);
	Offset *offset = NULL;
	if (is_random) offset = new RandomOffset(max_offsets);
	else           offset = new SequentialOffset(max_offsets);
	if (NULL == offset) {
		myprintf(__FILE__, __LINE__, ErrorSyscall, true, "malloc");
	}
	const uint64_t last = *offset;
	uint64_t next_status_print = STATUS_BLKCNT;
	do {
	       	for (uint16_t i = 0; i < qdepth; i++) {
			if (false == ios[i].used) {
				const uint64_t a = *offset;
				const uint64_t b =  a * iosize;
				const uint32_t c =  (b + iosize - 1 <= max_lba) ? iosize : max_lba - b + 1;
				if (is_write) {
					do_write(key, blocksize, b, c, ios[i]);
				} else {
					do_read( key, blocksize, b, c, ios[i]);
				}
				blocks_accessed += c;
			}
		}
		do_wait(key, fds, blocksize);
		if (blocks_accessed >= next_status_print) {
		       	if (logfp) gzprintf(logfp,  "%s UTC: 0x%08lX blocks accessed (%6.2lf%% done)\n", strtime().c_str(), blocks_accessed, double(blocks_accessed * 100) / double(max_lba + 1));
			else        fprintf(stderr, "%s UTC: 0x%08lX blocks accessed (%6.2lf%% done)\n", strtime().c_str(), blocks_accessed, double(blocks_accessed * 100) / double(max_lba + 1));
		       	next_status_print += STATUS_BLKCNT;
			break;
		}
	} while (last != offset->Next());

	delete offset;
	offset = NULL;
	delete [] ios;
	ios = NULL;

	for (std::set<int>::const_iterator i = fds.begin(); i != fds.end(); i++) {
	       	sg_cmds_close_device(*i);
	}

	if (data_miscompare) makeexit(-6);

	makeexit(0);
	return 0;
}

/*
 * Exit codes:
 *  0 : Normal Termination
 * -1 : caught signal (ErrorSignal)
 * -2 : error in command line parameters
 * -3 : error accessing the device
 * -4 : system call error (ErrorSyscall)
 * -5 :
 * -6 : data miscompare
 * -7 : cannot start slave processes
 * -8 : internal error (ErrorInternal)
 */
int main(int argc, char **argv) {

	if (SIG_ERR == signal(SIGHUP,    SIG_IGN)) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");
	if (SIG_ERR == signal(SIGINT,    getout )) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");
	if (SIG_ERR == signal(SIGQUIT,   getout )) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");
	if (SIG_ERR == signal(SIGILL,    getout )) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");
	if (SIG_ERR == signal(SIGABRT,   getout )) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");
	if (SIG_ERR == signal(SIGFPE,    getout )) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");
	if (SIG_ERR == signal(SIGSEGV,   getout )) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");
	if (SIG_ERR == signal(SIGPIPE,   getout )) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");
	if (SIG_ERR == signal(SIGALRM,   getout )) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");
	if (SIG_ERR == signal(SIGTERM,   getout )) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");
	if (SIG_ERR == signal(SIGUSR1,   SIG_IGN)) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");
	if (SIG_ERR == signal(SIGUSR2,   SIG_IGN)) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");
	if (SIG_ERR == signal(SIGCHLD,   SIG_IGN)) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");
	if (SIG_ERR == signal(SIGTTIN,   SIG_IGN)) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");
	if (SIG_ERR == signal(SIGTTOU,   SIG_IGN)) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");
	if (SIG_ERR == signal(SIGBUS,    getout )) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");
	if (SIG_ERR == signal(SIGPOLL,   getout )) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");
	if (SIG_ERR == signal(SIGPROF,   SIG_IGN)) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");
	if (SIG_ERR == signal(SIGSYS,    getout )) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");
	if (SIG_ERR == signal(SIGTRAP,   getout )) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");
	if (SIG_ERR == signal(SIGURG,    SIG_IGN)) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");
	if (SIG_ERR == signal(SIGVTALRM, getout )) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");
	if (SIG_ERR == signal(SIGXCPU,   getout )) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");
	if (SIG_ERR == signal(SIGXFSZ,   getout )) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");
	if (SIG_ERR == signal(SIGSTKFLT, getout )) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");
	if (SIG_ERR == signal(SIGIO,     SIG_IGN)) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");
	if (SIG_ERR == signal(SIGPWR,    getout )) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");
	if (SIG_ERR == signal(SIGWINCH,  SIG_IGN)) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");

	if (argc < 8) {
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

	const std::string logfile_prefix(argv[6]);

	std::vector<uint16_t> dnos;
	for (uint16_t i = 7; i < argc; i++) {
		if (std::string::npos == std::string(argv[i]).find_first_of("-")) {
		       	uint16_t dno;
		       	if (1 != sscanf(argv[i], "%hu", &dno)) {
			       	print_usage(argv[0]);
		       	}
		       	dnos.push_back(dno);
		} else if (((std::string(argv[i]).find_first_of("-") == std::string(argv[i]).find_last_of("-")))
			       	&& (0 != std::string(argv[i]).find_first_of("-"))
			       	&& (std::string(argv[i]).size() - 1 > std::string(argv[i]).find_last_of("-"))) {
			const std::string s(argv[i]);
			const std::string s1(s.substr(0, s.find_first_of("-")));
			const std::string s2(s.substr(s.find_first_of("-") + 1));
		       	uint16_t dno1;
		       	if (1 != sscanf(s1.c_str(), "%hu", &dno1)) {
			       	print_usage(argv[0]);
		       	}
		       	uint16_t dno2;
		       	if (1 != sscanf(s2.c_str(), "%hu", &dno2)) {
			       	print_usage(argv[0]);
		       	}
			if (dno1 > dno2) {
				const uint16_t x = dno1;
				dno1 = dno2;
				dno2 = x;
			}
			for (uint16_t j = dno1; j <= dno2; j++) {
			       	dnos.push_back(j);
			}
		} else {
		       	print_usage(argv[0]);
		}
	}

	if (1 < dnos.size()) {
		// The master process
		std::set<pid_t> spids;
		for (std::vector<uint16_t>::const_iterator i = dnos.begin(); i != dnos.end(); i++) {
			pid_t spid;
			switch (spid = fork()) {
				case -1:
					// fork failed
					if (0 < spids.size()) {
						// kill slaves
						for (std::set<pid_t>::const_iterator j = spids.begin(); j != spids.end(); j++) {
						       	if (kill(*j, SIGTERM)) {
							       	kill(*j, SIGKILL);
							}
						}
						for (std::set<pid_t>::const_iterator j = spids.begin(); j != spids.end(); j++) {
							int status = 0;
							if (*j != waitpid(*j, &status, 0)) {
								// wait failed for some reason
								std::ostringstream s;
								s << "waitpid(slave=" << *i << ",pid=" << *j << ")";
								myprintf(__FILE__, __LINE__, ErrorSyscall, false, s.str());
							}
						}
					}
					return -7;
				case 0:
					// slave
					return slave(argv[0], is_write, is_random, iosize, qdepth, key, logfile_prefix, *i);
				default:
					// master
					spids.insert(spid);
					break;
			}
		}
		// master tries to ignore all signals
		if (SIG_ERR == signal(SIGHUP,    SIG_IGN)) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");
		if (SIG_ERR == signal(SIGINT,    SIG_IGN)) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");
		if (SIG_ERR == signal(SIGQUIT,   SIG_IGN)) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");
		if (SIG_ERR == signal(SIGILL,    SIG_IGN)) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");
		if (SIG_ERR == signal(SIGABRT,   SIG_IGN)) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");
		if (SIG_ERR == signal(SIGFPE,    SIG_IGN)) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");
		if (SIG_ERR == signal(SIGSEGV,   SIG_IGN)) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");
		if (SIG_ERR == signal(SIGPIPE,   SIG_IGN)) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");
		if (SIG_ERR == signal(SIGALRM,   SIG_IGN)) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");
		if (SIG_ERR == signal(SIGTERM,   SIG_IGN)) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");
		if (SIG_ERR == signal(SIGUSR1,   SIG_IGN)) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");
		if (SIG_ERR == signal(SIGUSR2,   SIG_IGN)) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");
		if (SIG_ERR == signal(SIGCHLD,   SIG_DFL)) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal"); // to allow the master to get the return from the slaves
		if (SIG_ERR == signal(SIGTTIN,   SIG_IGN)) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");
		if (SIG_ERR == signal(SIGTTOU,   SIG_IGN)) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");
		if (SIG_ERR == signal(SIGBUS,    SIG_IGN)) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");
		if (SIG_ERR == signal(SIGPOLL,   SIG_IGN)) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");
		if (SIG_ERR == signal(SIGPROF,   SIG_IGN)) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");
		if (SIG_ERR == signal(SIGSYS,    SIG_IGN)) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");
		if (SIG_ERR == signal(SIGTRAP,   SIG_IGN)) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");
		if (SIG_ERR == signal(SIGURG,    SIG_IGN)) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");
		if (SIG_ERR == signal(SIGVTALRM, SIG_IGN)) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");
		if (SIG_ERR == signal(SIGXCPU,   SIG_IGN)) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");
		if (SIG_ERR == signal(SIGXFSZ,   SIG_IGN)) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");
		if (SIG_ERR == signal(SIGSTKFLT, SIG_IGN)) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");
		if (SIG_ERR == signal(SIGIO,     SIG_IGN)) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");
		if (SIG_ERR == signal(SIGPWR,    SIG_IGN)) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");
		if (SIG_ERR == signal(SIGWINCH,  SIG_IGN)) myprintf(__FILE__, __LINE__, ErrorSyscall, true, "signal");

		int retval = 0;
		for (std::set<pid_t>::const_iterator j = spids.begin(); j != spids.end(); j++) {
			int status = 0;
			pid_t x;
			if (*j != (x = waitpid(*j, &status, 0))) {
				// wait failed for some reason
				std::ostringstream s;
				s << "waitpid(pid=" << *j << ")";
				myprintf(__FILE__, __LINE__, ErrorSyscall, false, s.str());
				retval = -7;
			}
			if (!retval && status) {
				retval = status;
			}
		}
		return retval;
	}
       	return slave(argv[0], is_write, is_random, iosize, qdepth, key, logfile_prefix, dnos[0]);
}
