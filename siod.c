/*
 * This program accesses a SCSI generic device such that:
 * 1> Every block on the device is accessed exactly once.
 * 2> If the program is in write mode, the contents of every block will be
 *    unique after the program finishes. The contents of each block will
 *    also encode:
 *        a. The 'key' argument to the program
 *        b. The logical block address of the block
 *        c. The initiator address
 *        d. The target address
 */
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <string>
#include <iostream>
#include <vector>
#include <set>
#include <iomanip>
#include <sstream>

#include "lfsr.h"
#include "siod.h"

typedef struct tagIO {
    bool used;
    uint16_t me;
    double start, end;
    unsigned char cdb[CDB_SIZE];
    unsigned char sb[SENSE_LENGTH];
    int fd;
} IO;

extern gzFile   logfp;
extern uint64_t blocks_accessed;
extern uint64_t data_key_count[4];
static bool     data_miscompare = false;

static bool DeleteSHM(const key_t & shmkey) {
    // delete the shared memory segment to track slave progress
    const int shmd = shmget(shmkey, 0, 0);
    if (0 > shmd) {
        fprintf(stderr, "shmget: %s", strerror(errno));
        return true;
    }
    if (shmctl(shmd, IPC_RMID, NULL)) {
        fprintf(stderr, "shmctl(IPC_RMID): %s", strerror(errno));
        return true;
    }
    return false;
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

static unsigned char *RandomData(const uint8_t & key, const uint64_t & address,
        const uint16_t & blocksize, const uint16_t & blocks,
        const unsigned char * const iddata) {
    if (!blocks) {
        logprint(__FILE__, __LINE__, ErrorInternal, true,
                std::string("Buffer size error ") + std::to_string(blocks));
    }
    if (!iddata) {
        logprint(__FILE__, __LINE__, ErrorInternal, true,
                std::string("Identification data error "));
    }
    uint64_t x = 0;
    if ((!blocksize) || (blocksize % sizeof(x)) || (blocksize < 32)) {
        logprint(__FILE__, __LINE__, ErrorInternal, true,
                std::string("Block size error ") + std::to_string(blocksize));
    }
    unsigned char * const data = new unsigned char[blocksize * blocks];
    if (NULL == data) {
        logprint(__FILE__, __LINE__, ErrorSyscall, true, "malloc");
    }
    if (key) {
        const uint64_t mask = (uint64_t)(key & 0x3) << 62;
        for (uint16_t k = 0; k < blocks; k++) {
            LFSR lfsr(0x3FFFFFFFFFFFFFFFUL,
                    (address + k) ? (address + k) : (address + k - 1));
            for (uint16_t j = 0; j < (key & 0x3); j++) {
                x = lfsr++;
            }
            x = (x & 0x3FFFFFFFFFFFFFFFUL) | mask;
            for (uint16_t i = 0; i < blocksize; i += sizeof(x)) {
                if (data + k * blocksize + i !=
                        memcpy(data + k * blocksize + i, &x, sizeof(x))) {
                    logprint(__FILE__, __LINE__, ErrorSyscall, true, "memcpy");
                }
            }
            memcpy(data + k * blocksize + 8, iddata, 24);
        }
    } else {
        if (data != memset(data, 0, blocksize * blocks)) {
            logprint(__FILE__, __LINE__, ErrorSyscall, true, "memset");
        }
    }
    return data;
}

static bool WrongData(const uint8_t & key, const uint64_t & address,
        const uint16_t & blocksize, const uint16_t & blocks,
        const unsigned char * const data, const unsigned char * const iddata) {
    bool retval = false;
    bool error_range_f = false;
    uint64_t error_range_start = 0;
    uint64_t error_range_end = 0;
    if (key) {
        const unsigned char * const xdata = RandomData(key, address, blocksize,
                blocks, iddata);
        for (uint16_t j = 0; j < blocks; j++) {
            data_key_count[key] += 1;
            for (uint16_t i = 0; i < blocksize; i++) {
                if (xdata[j * blocksize + i] != data[j * blocksize + i]) {
                    retval = true;
                    data_key_count[0]   += 1;
                    data_key_count[key] -= 1;
                    if (error_range_f) {
                        if (address + j == error_range_end + 1) {
                            error_range_end = address + j;
                        } else {
                            std::ostringstream error_block;
                            error_block << UINT64(error_range_start).c_str()
                                << " " << UINT64(error_range_end).c_str();
                            logprint(__FILE__, __LINE__, ErrorBlock, false,
                                    error_block.str());
                            error_range_start = error_range_end = address + j;
                        }
                    } else {
                        error_range_f = true;
                        error_range_start = error_range_end = address + j;
                    }
                    break;
                }
            }
        }
        delete [] xdata;
    } else {
        for (uint16_t j = 0; j < blocks; j++) {
            const uint8_t bkey =
                ((*(uint64_t *)(data + j * blocksize)) >> 62) & 0x3;
            const unsigned char * const xdata =
                RandomData(bkey, address + j, blocksize, 1, iddata);
            data_key_count[bkey] += 1;
            for (uint16_t i = 0; i < blocksize; i++) {
                if (xdata[i] != data[j * blocksize + i]) {
                    retval = true;
                    data_key_count[0]    += 1;
                    data_key_count[bkey] -= 1;
                    if (error_range_f) {
                        if (address + j == error_range_end + 1) {
                            error_range_end = address + j;
                        } else {
                            std::ostringstream error_block;
                            error_block << UINT64(error_range_start).c_str()
                                << " " << UINT64(error_range_end).c_str();
                            logprint(__FILE__, __LINE__, ErrorBlock, false,
                                    error_block.str());
                            error_range_start = error_range_end = address + j;
                        }
                    } else {
                        error_range_f = true;
                        error_range_start = error_range_end = address + j;
                    }
                    break;
                }
            }
            delete [] xdata;
        }
    }
    // make sure the last range gets printed!
    if (error_range_f) {
        std::ostringstream error_block;
        error_block << UINT64(error_range_start).c_str() << " " <<
            UINT64(error_range_end).c_str();
        logprint(__FILE__, __LINE__, ErrorBlock, false, error_block.str());
    }
    return retval;
}

static void do_io(const uint8_t & key, const uint32_t & blocksize,
        const uint64_t & offset, const uint16_t & length, IO & io,
        const char & opcode, const int & dxfer_direction,
        const unsigned char * const iddata) {
    io.used = true;
    io.start = io.end = gettime();
    if (io.cdb != memset(io.cdb, 0, CDB_SIZE)) {
        logprint(__FILE__, __LINE__, ErrorSyscall, true, "memset");
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
        logprint(__FILE__, __LINE__, ErrorSyscall, true, "memset");
    }
    io_hdr.interface_id    = 'S';
    io_hdr.cmd_len         = CDB_SIZE;
    io_hdr.mx_sb_len       = SENSE_LENGTH;
    io_hdr.dxfer_direction = dxfer_direction;
    io_hdr.dxfer_len       = length * blocksize;
    if (SG_DXFER_FROM_DEV == io_hdr.dxfer_direction) {
        io_hdr.dxferp = new unsigned char[io_hdr.dxfer_len];
        if (NULL == io_hdr.dxferp) {
            logprint(__FILE__, __LINE__, ErrorSyscall, true, "malloc");
        }
        if (io_hdr.dxferp != memset(io_hdr.dxferp, 0, io_hdr.dxfer_len)) {
            logprint(__FILE__, __LINE__, ErrorSyscall, true, "memset");
        }
    } else {
        io_hdr.dxferp = RandomData(key, offset, blocksize, length, iddata);
    }
    io_hdr.cmdp            = io.cdb;
    io_hdr.sbp             = io.sb;
    io_hdr.timeout         = 5000;     /* 5000 millisecs == 5 seconds */
    io_hdr.flags           = SG_FLAG_LUN_INHIBIT | SG_FLAG_DIRECT_IO;
    io_hdr.pack_id         = io.me;
    io_hdr.usr_ptr         = &io;
    if (-1 == write(io.fd, &io_hdr, sizeof(sg_io_hdr_t))) {
        logprint(__FILE__, __LINE__, ErrorIO, false, "write", io.start, io.end,
                io.cdb);
        delete [] (unsigned char *)io_hdr.dxferp;
        io.used = false;
    }
}

static void do_write(const uint8_t & key, const uint32_t & blocksize,
        const uint64_t & offset, const uint16_t & length, IO & io,
        const unsigned char * const iddata) {
    do_io(key, blocksize, offset, length, io, 0x8A, SG_DXFER_TO_DEV, iddata);
}

static void do_read(const uint8_t & key, const uint32_t & blocksize,
        const uint64_t & offset, const uint16_t & length, IO & io,
        const unsigned char * const iddata) {
    do_io(key, blocksize, offset, length, io, 0x88, SG_DXFER_FROM_DEV, iddata);
}

static void do_wait(const uint8_t & key, const std::set<int> fds,
        const uint16_t & blocksize, const unsigned char * const iddata) {
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
            logprint(__FILE__, __LINE__, ErrorSyscall, true, "pselect");
            break;
        case 0:
            /* severe error, control should never come here */
            logprint(__FILE__, __LINE__, ErrorSyscall, true, "pselect");
            break;
        default:
            for (std::set<int>::const_iterator fd = fds.begin();
                    fd != fds.end(); fd++) {
                if (FD_ISSET(*fd, &FDS)) {
                    sg_io_hdr_t io_hdr;
                    if (&io_hdr != memset(&io_hdr, 0, sizeof(sg_io_hdr_t))) {
                        logprint(__FILE__, __LINE__, ErrorSyscall, true,
                                "memset");
                    }
                    if (-1 == read(*fd, &io_hdr, sizeof(sg_io_hdr_t))) {
                        logprint(__FILE__, __LINE__, ErrorSyscall, true,
                                "read");
                    } 
                    IO * const io = (IO *)(io_hdr.usr_ptr);
                    if (NULL == io) {
                        logprint(__FILE__, __LINE__, ErrorSyscall, true,
                                "usr_ptr");
                    } 
                    io->end = gettime();
                    io->used = false;
                    const std::string s(strtime());
                    const bool in_error = (io_hdr.masked_status != GOOD) ||
                        (io_hdr.host_status != SG_LIB_DID_OK) ||
                        (io_hdr.driver_status != SG_LIB_DRIVER_OK);
                    switch (io_hdr.masked_status) {
                        case GOOD:
                            break;
                        case CHECK_CONDITION:
                            logprint(__FILE__, __LINE__, ErrorIO, false,
                                    "Check Condition", io->start, io->end,
                                    io->cdb, io->sb);
                            break;
                        case CONDITION_GOOD:
                            logprint(__FILE__, __LINE__, ErrorIO, false,
                                    "Condition Good", io->start, io->end,
                                    io->cdb, io->sb);
                            break;
                        case INTERMEDIATE_GOOD:
                            logprint(__FILE__, __LINE__, ErrorIO, false,
                                    "Intermediate Good", io->start, io->end,
                                    io->cdb, io->sb);
                            break;
                        case INTERMEDIATE_C_GOOD:
                            logprint(__FILE__, __LINE__, ErrorIO, false,
                                    "Intermediate C Good", io->start, io->end,
                                    io->cdb, io->sb);
                            break;
                        case BUSY:
                            logprint(__FILE__, __LINE__, ErrorIO, false,
                                    "Busy", io->start, io->end, io->cdb,
                                    io->sb);
                            break;
                        case RESERVATION_CONFLICT:
                            logprint(__FILE__, __LINE__, ErrorIO, false,
                                    "Reservation Conflict", io->start, io->end,
                                    io->cdb, io->sb);
                            break;
                        case COMMAND_TERMINATED:
                            logprint(__FILE__, __LINE__, ErrorIO, false,
                                    "Command Terminated", io->start, io->end,
                                    io->cdb, io->sb);
                            break;
                        case QUEUE_FULL:
                            logprint(__FILE__, __LINE__, ErrorIO, false,
                                    "Queue Full", io->start, io->end, io->cdb,
                                    io->sb);
                            break;
                        default:
                            {
                                std::ostringstream s;
                                s << "Unknown Condition (0x" <<
                                    std::setfill('0') << std::hex <<
                                    std::setw(2) << io_hdr.masked_status;
                                logprint(__FILE__, __LINE__, ErrorIO, false,
                                        s.str(), io->start, io->end, io->cdb,
                                        io->sb);
                            }
                            break;
                    }
                    switch (io_hdr.host_status) {
                        case SG_LIB_DID_OK: 
                            break;
                        case SG_LIB_DID_NO_CONNECT: 
                            logprint(__FILE__, __LINE__, ErrorIO, false,
                                    "HOST UNABLE TO CONNECT BEFORE TIMEOUT",
                                    io->start, io->end, io->cdb, io->sb);
                            break;
                        case SG_LIB_DID_BUS_BUSY: 
                            logprint(__FILE__, __LINE__, ErrorIO, false,
                                    "HOST BUS BUSY TILL TIMEOUT", io->start,
                                    io->end, io->cdb, io->sb);
                            break;
                        case SG_LIB_DID_TIME_OUT: 
                            logprint(__FILE__, __LINE__, ErrorIO, false,
                                    "HOST TIMEOUT", io->start, io->end,
                                    io->cdb, io->sb);
                            break;
                        case SG_LIB_DID_BAD_TARGET: 
                            logprint(__FILE__, __LINE__, ErrorIO, false,
                                    "HOST BAD TARGET", io->start, io->end,
                                    io->cdb, io->sb);
                            break;
                        case SG_LIB_DID_ABORT: 
                            logprint(__FILE__, __LINE__, ErrorIO, false,
                                    "HOST ABORT", io->start, io->end, io->cdb,
                                    io->sb);
                            break;
                        case SG_LIB_DID_PARITY: 
                            logprint(__FILE__, __LINE__, ErrorIO, false,
                                    "HOST PARITY ERROR ON SCSI BUS", io->start,
                                    io->end, io->cdb, io->sb);
                            break;
                        case SG_LIB_DID_ERROR: 
                            logprint(__FILE__, __LINE__, ErrorIO, false,
                                    "HOST INTERNAL ERROR", io->start, io->end,
                                    io->cdb, io->sb);
                            break;
                        case SG_LIB_DID_RESET: 
                            logprint(__FILE__, __LINE__, ErrorIO, false,
                                    "HOST RESET", io->start, io->end, io->cdb,
                                    io->sb);
                            break;
                        case SG_LIB_DID_BAD_INTR: 
                            logprint(__FILE__, __LINE__, ErrorIO, false,
                                    "HOST RECEIVED AN UNEXPECTED  INTERRUPT",
                                    io->start, io->end, io->cdb, io->sb);
                            break;
                        case SG_LIB_DID_PASSTHROUGH: 
                            logprint(__FILE__, __LINE__, ErrorIO, false,
                                    "HOST FORCE COMMAND PAST MID-LEVEL",
                                    io->start, io->end, io->cdb, io->sb);
                            break;
                        case SG_LIB_DID_SOFT_ERROR: 
                            logprint(__FILE__, __LINE__, ErrorIO, false,
                                    "HOST THE LOW LEVEL DRIVER WANTS A RETRY",
                                    io->start, io->end, io->cdb, io->sb);
                            break;
                        default:
                            {
                                std::ostringstream s;
                                s << "HOST UNKNOWN STATUS (0x" <<
                                    std::setfill('0') << std::hex <<
                                    std::setw(2) << io_hdr.host_status;
                                logprint(__FILE__, __LINE__, ErrorIO, false,
                                        s.str(), io->start, io->end, io->cdb,
                                        io->sb);
                            }
                            break;
                    }
                    switch (io_hdr.driver_status & 0xF) {
                        case SG_LIB_DRIVER_OK: 
                            break;
                        case SG_LIB_DRIVER_BUSY: 
                            logprint(__FILE__, __LINE__, ErrorIO, false,
                                    "DRIVER BUSY", io->start, io->end, io->cdb,
                                    io->sb);
                            break;
                        case SG_LIB_DRIVER_SOFT: 
                            logprint(__FILE__, __LINE__, ErrorIO, false,
                                    "DRIVER SOFTWARE ERROR", io->start,
                                    io->end, io->cdb, io->sb);
                            break;
                        case SG_LIB_DRIVER_MEDIA: 
                            logprint(__FILE__, __LINE__, ErrorIO, false,
                                    "DRIVER MEDIA ERROR", io->start, io->end,
                                    io->cdb, io->sb);
                            break;
                        case SG_LIB_DRIVER_ERROR: 
                            logprint(__FILE__, __LINE__, ErrorIO, false,
                                    "DRIVER ERROR", io->start, io->end,
                                    io->cdb, io->sb);
                            break;
                        case SG_LIB_DRIVER_INVALID: 
                            logprint(__FILE__, __LINE__, ErrorIO, false,
                                    "DRIVER INVALID ERROR", io->start, io->end,
                                    io->cdb, io->sb);
                            break;
                        case SG_LIB_DRIVER_TIMEOUT: 
                            logprint(__FILE__, __LINE__, ErrorIO, false,
                                    "DRIVER TIMEOUT ERROR", io->start, io->end,
                                    io->cdb, io->sb);
                            break;
                        case SG_LIB_DRIVER_HARD: 
                            logprint(__FILE__, __LINE__, ErrorIO, false,
                                    "DRIVER HARDWARE ERROR", io->start,
                                    io->end, io->cdb, io->sb);
                            break;
                        case SG_LIB_DRIVER_SENSE: 
                            logprint(__FILE__, __LINE__, ErrorIO, false,
                                    "DRIVER SENSE ERROR", io->start, io->end,
                                    io->cdb, io->sb);
                            break;
                        default:
                            {
                                std::ostringstream s;
                                s << "DRIVER UNKNOWN STATUS (0x" <<
                                    std::setfill('0') << std::hex <<
                                    std::setw(2) << io_hdr.driver_status;
                                logprint(__FILE__, __LINE__, ErrorIO, false,
                                        s.str(), io->start, io->end, io->cdb,
                                        io->sb);
                            }
                            break;
                    }
                    if ((!in_error) &&
                            (SG_DXFER_FROM_DEV == io_hdr.dxfer_direction)) {
                        uint64_t address = 0;
                        for (unsigned int j = 0; j < 8; j++) {
                            address = (address << 8) | io_hdr.cmdp[2 + j];
                        }
                        uint16_t length = 0;
                        for (unsigned int j = 0; j < 2; j++) {
                            length = (length << 8) | io_hdr.cmdp[12 + j];
                        }
                        if (WrongData(key, address, blocksize, length,
                                    (unsigned char *)io_hdr.dxferp, iddata)) {
                            logprint(__FILE__, __LINE__, ErrorData, false, "",
                                    io->start, io->end, io->cdb);
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
    fprintf(stderr, "\nUsage:\n\n%s <operation> <locality> <I/O size> <queue \
depth> <encoding> <logfile prefix> <device>+\n\n", p.c_str());
    fprintf(stderr, "\tOperation   - [rw] read/write\n");
    fprintf(stderr, "\tLocality    - [rs] random/sequential\n");
    fprintf(stderr, "\tI/O size    - [0-9]+ number of blocks\n");
    fprintf(stderr, "\tQueue depth - [0-9]+ \n");
    fprintf(stderr, "\tEncoding    - [0123]\n");
    fprintf(stderr, "\t\t\t0 - Write -> all zeroes      Read -> detect \
encoding and check data (all zeros is invalid data)\n");
    fprintf(stderr, "\t\t\t1 - Write -> with encoding 1 Read -> check for \
encoding 1\n");
    fprintf(stderr, "\t\t\t2 - Write -> with encoding 2 Read -> check for \
encoding 2\n");
    fprintf(stderr, "\t\t\t3 - Write -> with encoding 3 Read -> check for \
encoding 3\n");
    fprintf(stderr, "\tDevice      - <sg>:<ha>:<ta>:<ts>\n");
    fprintf(stderr, "\t\t\tsg - [0-9]+ The sg device number, device XXX \
refers to /dev/sgXXX\n");
    fprintf(stderr, "\t\t\tha - [0-9a-fA-F]+ The host address as an 8 byte \
hexadecimal integer\n");
    fprintf(stderr, "\t\t\tta - [0-9a-fA-F]+ The target address as an 8 byte \
hexadecimal integer\n");
    fprintf(stderr, "\t\t\tts - [0-9]+ The time stamp as an 8 byte integer\n");
    fprintf(stderr, "\n");
    makeexit(-2);
}

static void getout(int s) {
    logprint(__FILE__, __LINE__, ErrorSignal, true, strsignal(s));
}

static int slave(const std::string & argv0, const bool & is_write,
        const bool & is_random, const uint16_t & iosize, uint16_t & qdepth,
        const uint8_t & key, const std::string & logfile_prefix,
        const uint16_t & dno, const unsigned char * const iddata,
        const uint16_t & rank, const key_t & shmkey) {
    data_key_count[0] = 0;
    data_key_count[1] = 0;
    data_key_count[2] = 0;
    data_key_count[3] = 0;

    const std::string device = std::string("/dev/sg") + std::to_string(dno);

    // the slave process
    {
        const std::string logfilename = logfile_prefix + std::string("log.") +
            std::to_string(dno) + std::string(".gz");
        logfp = gzopen(logfilename.c_str(), "wb");
        if (!logfp) logprint(__FILE__, __LINE__, ErrorSyscall, false,
                "Cannot open log file");
        if (logfp) gzbuffer(logfp, 256 * 1024);
    }

    char * shmp = NULL;
    {
        const int shmd = shmget(shmkey, 0, 0);
        if (0 > shmd) {
            logprint(__FILE__, __LINE__, ErrorSyscall, false, "shmget");
        } else {
            shmp = (char *)shmat(shmd, NULL, 0);
            if ((void *)-1 == (void *)shmp) {
                logprint(__FILE__, __LINE__, ErrorSyscall, false, "shmat");
                shmp = NULL;
            }
        }
    }

    IO *ios = new IO[qdepth];
    if (NULL == ios) {
        logprint(__FILE__, __LINE__, ErrorSyscall, true, "malloc");
    }
    std::set<int> fds;
    {
        int fd = -1;
        for (uint16_t i = 0; i < qdepth; i++) {
            if ((0 == i % SG_MAX_QUEUE) || (fd == -1)) {
                fd = sg_cmds_open_device(device.c_str(), 0, 0);
                if (fd < 0) {
                    logprint(__FILE__, __LINE__, ErrorSyscall, true, "open");
                }
                if (false == fds.insert(fd).second) {
                    logprint(__FILE__, __LINE__, ErrorInternal, true,
                            "set insert");
                }
            }
            if (&ios[i] != memset(&ios[i], 0, sizeof(IO))) {
                logprint(__FILE__, __LINE__, ErrorSyscall, true, "memset");
            }
            ios[i].used = false;
            ios[i].me = i;
            ios[i].fd = fd;
        }
        std::string sno("XXXXXXXX");
        {
            unsigned char resp[0x18];
            bzero(resp, sizeof(resp));
            switch (int retval = sg_ll_inquiry(fd, 0, 1, 0x80, resp, sizeof(resp), 0, 0)) {
                case 0:
                    sno = "";
                    for (uint16_t i = 0x4; i < (0x4 + resp[3]); i++) {
                        sno += resp[i];
                    }
                    break;
                case SG_LIB_CAT_INVALID_OP:
                    logprint(__FILE__, __LINE__, ErrorSyscall, false,
                       "sg_ll_inquiry(SG_LIB_CAT_INVALID_OP) : not supported");
                    break;
                case SG_LIB_CAT_ILLEGAL_REQ:
                    logprint(__FILE__, __LINE__, ErrorSyscall, false,
                   "sg_ll_inquiry(SG_LIB_CAT_ILLEGAL_REQ) : bad field in cdb");
                    break;
                case SG_LIB_CAT_ABORTED_COMMAND:
                    logprint(__FILE__, __LINE__, ErrorSyscall, false,
                "sg_ll_inquiry(SG_LIB_CAT_ABORTED_COMMAND) : aborted command");
                    break;
                default:
                    {
                        std::ostringstream o;
                        o << "sg_ll_inquiry(0x" << std::setfill('0') <<
                            std::hex << std::setw(2) <<retval <<
                            ") : unknown error";
                        logprint(__FILE__, __LINE__, ErrorSyscall, false,
                                o.str());
                        break;
                    }
               }
        }
        if (logfp) gzprintf(logfp,  "%s %s %c %c %hu %hu %hu %s %s %s\n",
                LoglineStart().c_str(), argv0.c_str(), is_write ? 'W' : 'R',
                is_random ? 'R' : 'S', iosize, qdepth, key,
                logfile_prefix.c_str(), device.c_str(), sno.c_str());
        else       fprintf(stderr, "%s %s %c %c %hu %hu %hu %s %s %s\n",
                LoglineStart().c_str(), argv0.c_str(), is_write ? 'W' : 'R',
                is_random ? 'R' : 'S', iosize, qdepth, key,
                logfile_prefix.c_str(), device.c_str(), sno.c_str());
    }

    uint64_t max_lba = 0;
    uint32_t blocksize = 0;
    {
        unsigned char resp[32];
        if (resp != memset(resp, 0, sizeof(resp))) {
            logprint(__FILE__, __LINE__, ErrorSyscall, true, "memset");
        }
        switch (int retval = sg_ll_readcap_16(ios[0].fd, 0, 0, resp, 32, 0, 0)) {
            case 0:
                break;
            case SG_LIB_CAT_UNIT_ATTENTION:
                logprint(__FILE__, __LINE__, ErrorSyscall, true,
                "sg_ll_readcap_16(SG_LIB_CAT_UNIT_ATTENTION): unit attention");
                break;
            case SG_LIB_CAT_INVALID_OP:
                logprint(__FILE__, __LINE__, ErrorSyscall, true,
                "sg_ll_readcap_16(SG_LIB_CAT_INVALID_OP): cdb not supported");
                break;
            case SG_LIB_CAT_ILLEGAL_REQ:
                logprint(__FILE__, __LINE__, ErrorSyscall, true,
                "sg_ll_readcap_16(SG_LIB_CAT_IlLEGAL_REQ): bad field in cdb");
                break;
            case SG_LIB_CAT_NOT_READY:
                logprint(__FILE__, __LINE__, ErrorSyscall, true,
                "sg_ll_readcap_16(SG_LIB_CAT_NOT_READY): device not ready");
                break;
            case SG_LIB_CAT_ABORTED_COMMAND:
                logprint(__FILE__, __LINE__, ErrorSyscall, true, 
                "sg_ll_readcap_16(SG_LIB_CAT_ABORTED_COMMAND): aborted command");
                break;
            case -1:
                logprint(__FILE__, __LINE__, ErrorSyscall, true,
                        "sg_ll_readcap_16(-1): other error");
                break;
            default:
                {
                    std::ostringstream o;
                    o << "sg_ll_readcap_16(0x" << std::setfill('0') <<
                        std::hex << std::setw(2) <<retval <<
                        ") : unknown error";
                    logprint(__FILE__, __LINE__, ErrorSyscall, true, o.str());
                }
                break;
        }
        for (unsigned int i = 0; i < 8; i++) {
            max_lba = (max_lba << 8) + resp[i];
        }
        for (unsigned int i = 8; i < 12; i++) {
            blocksize = (blocksize << 8) + resp[i];
        }
    }
    if (logfp) gzprintf(logfp,  "%s Max LBA %s Block Size %u\n",
            LoglineStart().c_str(), UINT64(max_lba).c_str(), blocksize);
    else        fprintf(stderr, "%s Max LBA %s Block Size %u\n",
            LoglineStart().c_str(), UINT64(max_lba).c_str(), blocksize);

    const uint64_t max_offsets = (max_lba + 1) / iosize +
        (((max_lba + 1) % iosize) ? 1 : 0);
    Offset *offset = NULL;
    if (is_random) offset = new RandomOffset(max_offsets);
    else           offset = new SequentialOffset(max_offsets);
    if (NULL == offset) {
        logprint(__FILE__, __LINE__, ErrorSyscall, true, "malloc");
    }
    const uint64_t last = *offset;
    uint64_t next_status_print = STATUS_BLKCNT;
    do {
        for (uint16_t i = 0; i < qdepth; i++) {
            if (false == ios[i].used) {
                const uint64_t a = *offset;
                const uint64_t b =  a * iosize;
                const uint32_t c =  (b + iosize - 1 <= max_lba) ?
                    iosize : max_lba - b + 1;
                if (is_write) {
                    do_write(key, blocksize, b, c, ios[i], iddata);
                } else {
                    do_read( key, blocksize, b, c, ios[i], iddata);
                }
                blocks_accessed += c;
                offset->Next();
                if (last == *offset) break;
            }
        }
        do_wait(key, fds, blocksize, iddata);
        if (blocks_accessed >= next_status_print) {
            const double pdone = double(blocks_accessed * 100) /
                double(max_lba + 1);
            if (logfp) gzprintf(logfp, 
                    "%s %s blocks accessed (%6.2lf%% done) %s\n",
                    LoglineStart().c_str(), UINT64(blocks_accessed).c_str(),
                    pdone, KeyCounts().c_str());
            else        fprintf(stderr,
                    "%s %s blocks accessed (%6.2lf%% done) %s\n",
                    LoglineStart().c_str(), UINT64(blocks_accessed).c_str(),
                    pdone, KeyCounts().c_str());
            if (shmp) shmp[rank] = uint8_t(pdone);
            next_status_print += STATUS_BLKCNT;
        }
    } while (last != *offset);
       
    while (1) {
        bool done = true;
        for (uint16_t i = 0; i < qdepth; i++) {
            if (ios[i].used) {
                done = false;
                break;
            }
        }
        if (done) break;
        do_wait(key, fds, blocksize, iddata);
    }

    delete offset;
    offset = NULL;
    delete [] ios;
    ios = NULL;

    for (std::set<int>::const_iterator i = fds.begin(); i != fds.end(); i++) {
        sg_cmds_close_device(*i);
    }

    if (shmp) {
        shmp[rank] = 0xFF;
        if (shmdt(shmp)) {
            logprint(__FILE__, __LINE__, ErrorSyscall, false, "shmdt");
        }
    }

    if (data_miscompare) makeexit(-6);

    makeexit(0);
    return 0;
}

static void slave_signal_assignment() {
    if (SIG_ERR == signal(SIGHUP,    SIG_IGN)) logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
    if (SIG_ERR == signal(SIGINT,    getout )) logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
    if (SIG_ERR == signal(SIGQUIT,   getout )) logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
    if (SIG_ERR == signal(SIGILL,    getout )) logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
    if (SIG_ERR == signal(SIGABRT,   getout )) logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
    if (SIG_ERR == signal(SIGFPE,    getout )) logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
    if (SIG_ERR == signal(SIGSEGV,   getout )) logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
    if (SIG_ERR == signal(SIGPIPE,   getout )) logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
    if (SIG_ERR == signal(SIGALRM,   getout )) logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
    if (SIG_ERR == signal(SIGTERM,   getout )) logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
    if (SIG_ERR == signal(SIGUSR1,   SIG_IGN)) logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
    if (SIG_ERR == signal(SIGUSR2,   SIG_IGN)) logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
    // if (SIG_ERR == signal(SIGCHLD,   SIG_IGN)) logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
    if (SIG_ERR == signal(SIGTTIN,   SIG_IGN)) logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
    if (SIG_ERR == signal(SIGTTOU,   SIG_IGN)) logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
    if (SIG_ERR == signal(SIGBUS,    getout )) logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
    if (SIG_ERR == signal(SIGPOLL,   getout )) logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
    if (SIG_ERR == signal(SIGPROF,   SIG_IGN)) logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
    if (SIG_ERR == signal(SIGSYS,    getout )) logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
    if (SIG_ERR == signal(SIGTRAP,   getout )) logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
    if (SIG_ERR == signal(SIGURG,    SIG_IGN)) logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
    if (SIG_ERR == signal(SIGVTALRM, getout )) logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
    if (SIG_ERR == signal(SIGXCPU,   getout )) logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
    if (SIG_ERR == signal(SIGXFSZ,   getout )) logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
    if (SIG_ERR == signal(SIGSTKFLT, getout )) logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
    if (SIG_ERR == signal(SIGIO,     SIG_IGN)) logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
    if (SIG_ERR == signal(SIGPWR,    getout )) logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
    if (SIG_ERR == signal(SIGWINCH,  SIG_IGN)) logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
}

static void master_signal_assignment() {
        // master tries to ignore all signals
        if (SIG_ERR == signal(SIGHUP,    SIG_IGN))
            logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
        if (SIG_ERR == signal(SIGINT,    SIG_IGN))
            logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
        if (SIG_ERR == signal(SIGQUIT,   SIG_IGN))
            logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
        if (SIG_ERR == signal(SIGILL,    SIG_IGN))
            logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
        if (SIG_ERR == signal(SIGABRT,   SIG_IGN))
            logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
        if (SIG_ERR == signal(SIGFPE,    SIG_IGN))
            logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
        if (SIG_ERR == signal(SIGSEGV,   SIG_IGN))
            logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
        if (SIG_ERR == signal(SIGPIPE,   SIG_IGN))
            logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
        if (SIG_ERR == signal(SIGALRM,   SIG_IGN))
            logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
        if (SIG_ERR == signal(SIGTERM,   SIG_IGN))
            logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
        if (SIG_ERR == signal(SIGUSR1,   SIG_IGN))
            logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
        if (SIG_ERR == signal(SIGUSR2,   SIG_IGN))
            logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
        if (SIG_ERR == signal(SIGCHLD,   SIG_DFL))
            logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal"); // to allow the master to get the return from the slaves
        if (SIG_ERR == signal(SIGTTIN,   SIG_IGN))
            logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
        if (SIG_ERR == signal(SIGTTOU,   SIG_IGN))
            logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
        if (SIG_ERR == signal(SIGBUS,    SIG_IGN))
            logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
        if (SIG_ERR == signal(SIGPOLL,   SIG_IGN))
            logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
        if (SIG_ERR == signal(SIGPROF,   SIG_IGN))
            logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
        if (SIG_ERR == signal(SIGSYS,    SIG_IGN))
            logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
        if (SIG_ERR == signal(SIGTRAP,   SIG_IGN))
            logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
        if (SIG_ERR == signal(SIGURG,    SIG_IGN))
            logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
        if (SIG_ERR == signal(SIGVTALRM, SIG_IGN))
            logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
        if (SIG_ERR == signal(SIGXCPU,   SIG_IGN))
            logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
        if (SIG_ERR == signal(SIGXFSZ,   SIG_IGN))
            logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
        if (SIG_ERR == signal(SIGSTKFLT, SIG_IGN))
            logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
        if (SIG_ERR == signal(SIGIO,     SIG_IGN))
            logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
        if (SIG_ERR == signal(SIGPWR,    SIG_IGN))
            logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
        if (SIG_ERR == signal(SIGWINCH,  SIG_IGN))
            logprint(__FILE__, __LINE__, ErrorSyscall, true, "signal");
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

    slave_signal_assignment();

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

    typedef struct tagDINFO {
        uint16_t sgno;
        uint64_t host_address;
        uint64_t target_address;
        uint64_t timestamp;
    } DINFO;

    std::vector<DINFO> dnos;
    for (uint16_t i = 7; i < argc; i++) {
        DINFO dno;
        if (4 != sscanf(argv[i], "%hu:%lx:%lx:%lu", &dno.sgno,
                    &dno.host_address, &dno.target_address, &dno.timestamp)) {
            print_usage(argv[0]);
        }
        dnos.push_back(dno);
    }

    {
        // create a lock file in /tmp
        const int fd = open(lockfile, O_WRONLY | O_CREAT | O_EXCL, S_IRUSR |
                S_IWUSR | S_IRGRP | S_IROTH);
        if (0 > fd) {
            fprintf(stderr, "lockfile(open): %s\n", strerror(errno));
            return -4;
        }
        FILE * const fp = fdopen(fd, "w");
        if (NULL == fp) {
            fprintf(stderr, "lockfile(fdopen): %s\n", strerror(errno));
            unlink(lockfile);
            return -4;
        }
        fprintf(fp, "%d\n", getpid());
        if (fclose(fp)) {
            fprintf(stderr, "lockfile(fclose): %s\n", strerror(errno));
            unlink(lockfile);
            return -4;
        }
    }

    const key_t shmkey = getpid();
    {
        // create a shared memory segment to track slave progress
        const int shmd = shmget(shmkey, dnos.size(), IPC_CREAT | IPC_EXCL |
                S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (0 > shmd) {
            fprintf(stderr, "shmget: %s", strerror(errno));
            unlink(lockfile);
            return -4;
        }
        void * const shm_status = shmat(shmd, NULL, 0);
        if ((void *)-1 == shm_status) {
            fprintf(stderr, "shmat: %s", strerror(errno));
            unlink(lockfile);
            DeleteSHM(shmkey);
            return -4;
        }
        bzero(shm_status, dnos.size());
        if (shmdt(shm_status)) {
            fprintf(stderr, "shmdt: %s", strerror(errno));
            unlink(lockfile);
            DeleteSHM(shmkey);
            return -4;
        }
    }

    // The master process
    std::set<pid_t> spids;
    uint16_t rank = 0;
    for (std::vector<DINFO>::const_iterator i = dnos.begin(); i != dnos.end();
            i++) {
        pid_t spid;
        switch (spid = fork()) {
            case -1:
                // fork failed
                if (0 < spids.size()) {
                    // kill slaves
                    for (std::set<pid_t>::const_iterator j = spids.begin();
                            j != spids.end(); j++) {
                        if (kill(*j, SIGTERM)) {
                            kill(*j, SIGKILL);
                        }
                    }
                    for (std::set<pid_t>::const_iterator j = spids.begin();
                            j != spids.end(); j++) {
                        int status = 0;
                        if (*j != waitpid(*j, &status, 0)) {
                            // wait failed for some reason
                            std::ostringstream s;
                            s << "waitpid(slave=" << i->sgno << ",pid=" << *j
                                << ")";
                            logprint(__FILE__, __LINE__, ErrorSyscall, false,
                                    s.str());
                        }
                    }
                }
                unlink(lockfile);
                DeleteSHM(shmkey);
                return -7;
            case 0:
                // slave
                unsigned char iddata[24];
                memcpy(iddata, &(i->host_address), sizeof(uint64_t));
                memcpy(iddata + sizeof(uint64_t), &(i->target_address),
                        sizeof(uint64_t));
                memcpy(iddata + 2 * sizeof(uint64_t), &(i->target_address),
                        sizeof(uint64_t));
                return slave(argv[0], is_write, is_random, iosize, qdepth, key,
                        logfile_prefix, i->sgno, iddata, rank, shmkey);
            default:
                // master
                spids.insert(spid);
                break;
        }
        rank++;
    }

    master_signal_assignment();

    int retval = 0;
    for (std::set<pid_t>::const_iterator j = spids.begin(); j != spids.end();
            j++) {
        int status = 0;
        pid_t x;
        if (*j != (x = waitpid(*j, &status, 0))) {
            // wait failed for some reason
            std::ostringstream s;
            s << "waitpid(pid=" << *j << ")";
            logprint(__FILE__, __LINE__, ErrorSyscall, false, s.str());
            retval = -7;
        }
        if (!retval && status) {
            retval = status;
        }
    }
    if (unlink(lockfile)) {
        fprintf(stderr, "lockfile(unlink): %s", strerror(errno));
        return -4;
    }
    if (DeleteSHM(shmkey)) {
        return (retval ? retval : -4);
    }
    return retval;
}
