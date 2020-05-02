#include <zlib.h>
#include <string.h>

#include <string>
#include <vector>
#include <sstream>
#include <iostream>

#include "siod.h"

static void print_usage(const std::string & p) {
    fprintf(stderr, "\nUsage:\n\n%s <logfile prefix> <device>+\n\n", p.c_str());
    fprintf(stderr, "\tdevice - [0-9]+ The sg device number, device XXX \
refers to /dev/sgXXX\n");
    fprintf(stderr, "\n");
    exit(-2);
}

void logprint(const char * const file, const int & line,
        const ErrorType & e, const bool & fatal,
        const std::string & s, const double & start,
        const double & end, const unsigned char * const cdb,
        const unsigned char * const sbp) {
    std::ostringstream head;
    head << LoglineStart() << file << ':' << line << ": ";
    switch (e) {
        case ErrorSyscall:
            {
                std::string errmsg(strerror(errno));
                fprintf(stderr, "%s Syscall error : %s (%s)\n",
			       	head.str().c_str(), s.c_str(), errmsg.c_str());
            }
            if (fatal) exit(-4);
            break;
        case ErrorInternal:
            fprintf(stderr, "%s Internal error : %s\n",
			    head.str().c_str(), s.c_str());
            if (fatal) exit(-8);
            break;
        case ErrorSignal:
            fprintf(stderr, "%s Signal error : %s\n",
			    head.str().c_str(), s.c_str());
            if (fatal) exit(-1);
            break;
        case ErrorIO:
            {
                std::string errmsg(strerror(errno));
                fprintf(stderr, "%s IO error : %s (%s) : %lf %lf : %s : %s\n",
                        head.str().c_str(), s.c_str(), errmsg.c_str(), start,
                        end, cdb2str(cdb).c_str(), sense2str(sbp).c_str());
            }
            break;
        case ErrorData:
            fprintf(stderr, "%s Data error : %lf %lf : %s\n",
			    head.str().c_str(), start, end, cdb2str(cdb).c_str());
            break;
        case ErrorBlock:
            fprintf(stderr, "%s Data Mismatch : %s\n",
			    head.str().c_str(), s.c_str());
            break;
        default:
            fprintf(stderr, "%s Unknown error\n",
			    head.str().c_str());
            break;
    }
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

    if (argc < 3) {
        print_usage(argv[0]);
    }

    const std::string logfile_prefix(argv[1]);

    std::vector<uint16_t> sgnos;
    for (uint16_t i = 2; i < argc; i++) {
	uint16_t sgno;
        if (1 != sscanf(argv[i], "%hu", &sgno)) {
	       	print_usage(argv[0]);
	}
        sgnos.push_back(sgno);
    }

    int retval = 0;

    for (std::vector<uint16_t>::const_iterator sgno = sgnos.begin(); sgno != sgnos.end(); sgno++) {
        const std::string logfilename = logfile_prefix + std::string("log.") +
            std::to_string(*sgno) + std::string(".gz");
       	gzFile logfp = gzopen(logfilename.c_str(), "r");
        if (!logfp) logprint(__FILE__, __LINE__, ErrorSyscall, true, "Cannot open log file");
        gzbuffer(logfp, 256 * 1024);
	   	while (!gzeof(logfp)) {
		   	char buffer[4096];
			if (gzgets(logfp, buffer, sizeof(buffer)) != buffer) {
			    if (gzeof(logfp)) break;
			   	logprint(__FILE__, __LINE__, ErrorSyscall, true, "Cannot read log file");
			}
			const std::string line(buffer);
			std::cout << line;
	   	}
        switch (gzclose(logfp)) {
            case Z_OK:
                break;
            case Z_STREAM_ERROR:
                fprintf(stderr, "%s GZIP error : file was NULL (or Z_NULL), \
or did not refer to an open compressed file stream\n",
                        LoglineStart().c_str());
                break;
            case Z_ERRNO:
                fprintf(stderr, "%s GZIP error : %s\n", LoglineStart().c_str(),
                        strerror(errno));
                break;
            case Z_BUF_ERROR:
                fprintf(stderr, "%s GZIP error : no compression progress is \
possible during buffer flush\n",
                        LoglineStart().c_str());
                break;
            default:
                fprintf(stderr, "%s GZIP error : unknown error\n",
                        LoglineStart().c_str());
                break;
        }
    }

    return retval;
}
