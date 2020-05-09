#include <string.h>
#include <zlib.h>
#include <sstream>
#include <iomanip>
#include "siod.h"

gzFile   logfp           = 0;
uint64_t blocks_accessed = 0;
uint64_t data_key_count[4];

std::string buffer2str(const unsigned char * const buf,
        const uint16_t & sz) {
    if (!buf || !sz) return std::string();
    std::string retval;
    for (uint16_t i = 0; i < sz; i++) {
        char str[6];
        if (0 > snprintf(str, 6, "0x%02X ", ((uint8_t)buf[i]))) {
            logprint(__FILE__, __LINE__, ErrorSyscall, true, "snprintf");
        }
        retval += str;
    }
    return retval;
}

std::string cdb2str(const unsigned char * const cdb) {
    return buffer2str(cdb, CDB_SIZE);
}

std::string sense2str(const unsigned char * const sense) {
    return buffer2str(sense, SENSE_LENGTH);
}

std::string LoglineStart() {
    std::ostringstream o;
    o << strtime() << " UTC:";
    return o.str();
}

std::string strtime() {
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

std::string UINT64(const uint64_t & x) {
    std::ostringstream o;
    o << "0x" << std::setfill('0') << std::hex << std::setw(16) << x;
    return o.str();
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
                if (logfp) gzprintf(logfp, "%s Syscall error : %s (%s)\n",
                        head.str().c_str(), s.c_str(), errmsg.c_str());
                else       fprintf(stderr, "%s Syscall error : %s (%s)\n",
                        head.str().c_str(), s.c_str(), errmsg.c_str());
            }
            if (fatal) makeexit(-4);
            break;
        case ErrorInternal:
            if (logfp) gzprintf(logfp,  "%s Internal error : %s\n",
                    head.str().c_str(), s.c_str());
            else       fprintf(stderr, "%s Internal error : %s\n",
                    head.str().c_str(), s.c_str());
            if (fatal) makeexit(-8);
            break;
        case ErrorSignal:
            if (logfp) gzprintf(logfp,  "%s Signal error : %s\n",
                    head.str().c_str(), s.c_str());
            else        fprintf(stderr, "%s Signal error : %s\n",
                    head.str().c_str(), s.c_str());
            if (fatal) makeexit(-1);
            break;
        case ErrorIO:
            {
                std::string errmsg(strerror(errno));
                if (logfp) gzprintf(logfp, 
                        "%s IO error : %s (%s) : %lf %lf : %s : %s\n",
                        head.str().c_str(), s.c_str(), errmsg.c_str(), start,
                        end, cdb2str(cdb).c_str(), sense2str(sbp).c_str());
                else        fprintf(stderr,
                        "%s IO error : %s (%s) : %lf %lf : %s : %s\n",
                        head.str().c_str(), s.c_str(), errmsg.c_str(), start,
                        end, cdb2str(cdb).c_str(), sense2str(sbp).c_str());
            }
            break;
        case ErrorData:
            if (logfp) gzprintf(logfp,  "%s Data error : %lf %lf : %s\n",
                    head.str().c_str(), start, end, cdb2str(cdb).c_str());
            else        fprintf(stderr, "%s Data error : %lf %lf : %s\n",
                    head.str().c_str(), start, end, cdb2str(cdb).c_str());
            break;
        case ErrorBlock:
            if (logfp) gzprintf(logfp,  "%s Data Mismatch : %s\n",
                    head.str().c_str(), s.c_str());
            else        fprintf(stderr, "%s Data Mismatch : %s\n",
                    head.str().c_str(), s.c_str());
            break;
        default:
            if (logfp) gzprintf(logfp,  "%s Unknown error\n",
                    head.str().c_str());
            else        fprintf(stderr, "%s Unknown error\n",
                    head.str().c_str());
            break;
    }
}

void makeexit(const int & status) {
    if (logfp) gzprintf(logfp,  "%s %s blocks accessed %s\n",
            LoglineStart().c_str(), UINT64(blocks_accessed).c_str(),
            KeyCounts().c_str());
    else       fprintf(stderr, "%s %s blocks accessed %s\n",
            LoglineStart().c_str(), UINT64(blocks_accessed).c_str(),
            KeyCounts().c_str());
    if (logfp) 
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
    exit(status);
}


std::string KeyCounts() {
    std::ostringstream o;
    o << UINT64(data_key_count[0]) << ' ';
    o << UINT64(data_key_count[1]) << ' ';
    o << UINT64(data_key_count[2]) << ' ';
    o << UINT64(data_key_count[3]);
    return o.str();
}
