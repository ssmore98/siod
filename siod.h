#ifndef SIOD_H
#define SIOD_H

#include <string>

#define CDB_SIZE     ((uint16_t)16)
#define SENSE_LENGTH ((uint16_t)32)
#define STATUS_BLKCNT ((uint64_t)2 * 1024 * 1024)

typedef enum {ErrorNone, ErrorSyscall, ErrorIO, ErrorData, ErrorBlock, ErrorSignal, ErrorInternal}
ErrorType;

static const char * const lockfile = "/tmp/siod.lck";

void logprint(const char * const file, const int & line,
        const ErrorType & e, const bool & fatal,
        const std::string & s = std::string(), const double & start = 0,
        const double & end = 0, const unsigned char * const cdb = NULL,
        const unsigned char * const sbp = NULL);
std::string buffer2str(const unsigned char * const buf, const uint16_t & sz);
std::string cdb2str(const unsigned char * const cdb);
std::string sense2str(const unsigned char * const sense);
std::string LoglineStart();
std::string UINT64(const uint64_t & x);
std::string strtime();
void logprint(const char * const file, const int & line,
        const ErrorType & e, const bool & fatal,
        const std::string & s, const double & start,
        const double & end, const unsigned char * const cdb,
        const unsigned char * const sbp);
void makeexit(const int & status);
std::string KeyCounts();


#endif
