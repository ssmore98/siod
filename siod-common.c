#include <sstream>
#include <iomanip>
#include "siod.h"

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


