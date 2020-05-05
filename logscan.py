import gzip
import re
import string

regex_log_day = '(Sun|Mon|Tue|Wed|Thu|Fri|Sat)'
regex_log_month = '(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec)'
regex_log_date = '( \d|\d{2})'
regex_log_time = '\d{2}:\d{2}:\d{2}'
regex_log_year = '\d{4}'
regex_log_date = '({0} {1} {2} {3} {4} UTC)'.format(
    regex_log_day, regex_log_month, regex_log_date, regex_log_time, regex_log_year)
regex_log_uint64 = '0x([\dA-Fa-f]{16})'
regex_log_blka = '{0} blocks accessed'.format(regex_log_uint64)
regex_log_perc = '(100| [1-9]\d|  \d)\.\d\d%'
regex_log_fname = '(lfsr\.h|siod\.c)'
regex_log_hexb = '0x[\dA-F]{2}'
regex_log_cdb = '({0} ){1}{0}'.format(regex_log_hexb, '{15}')
regex_log_sense = '({0} ){1}{0}'.format(regex_log_hexb, '{31}')
regex_log_dtime = '\d+\.\d+'
regex_log_se = '{0} {1}'.format(regex_log_dtime, regex_log_dtime)
regex_logline_drivesize = '^{0}: Max LBA {1} Block Size (512|520|4096)$'.format(
    regex_log_date, regex_log_uint64)
regex_logline_lastline = '^{0}: {1} {2} {3} {4} {5}$'.format(
    regex_log_date, regex_log_blka, regex_log_uint64, regex_log_uint64, regex_log_uint64, regex_log_uint64)
regex_logline_status = '^{0}: {1} \({2} done\) {3} {4} {5} {6}$'.format(
    regex_log_date, regex_log_blka, regex_log_perc, regex_log_uint64, regex_log_uint64, regex_log_uint64, regex_log_uint64)
regex_logline_esyscall = '^{0}:{1}:\d+:  Syscall error : (.+) \((.+)\)$'.format(
    regex_log_date, regex_log_fname)
regex_logline_esignal = '^{0}:{1}:\d+:  Signal error : (.+)$'.format(
    regex_log_date, regex_log_fname)
regex_logline_eintern = '^{0}:{1}:\d+:  Internal error : (.+)$'.format(
    regex_log_date, regex_log_fname)
regex_logline_edata = '^{0}:{1}:\d+:  Data error : {2} : ({3})$'.format(
    regex_log_date, regex_log_fname, regex_log_se, regex_log_cdb)
regex_logline_dmism = '^{0}:{1}:\d+:  Data Mismatch : {2} {2}$'.format(
    regex_log_date, regex_log_fname, regex_log_uint64)
regex_logline_eio = '^{0}:{1}:\d+:  IO error : (.+) \((.+)\) : {2} : ({3})  :( {4})?$'.format(
    regex_log_date, regex_log_fname, regex_log_se, regex_log_cdb, regex_log_sense)
regex_logline_eunknown = '^{0}:{1}:\d+:  Unknown error$'.format(
    regex_log_date, regex_log_fname)
regex_logline_eferror = '^Error return from the simple I/O driver binary.$'


def RegexLoglineCommand():
    return '^{0}: \S*siod (R|W) (R|S) \d+ \d+ [0123] \S+ /dev/sg\d+\s+\S+$'.format(regex_log_date)


def ScanLog(fp):
    regex_logline_command = RegexLoglineCommand()
    last_block = -1
    blocks_accessed = -1
    for line in fp:
        line = line.decode().strip()
        line = re.sub(r'[^{0}].*$'.format(string.printable), r'', line)
        if re.match(regex_logline_command, line):
            pass
        elif re.match(regex_logline_drivesize, line):
            last_block = int(
                re.match(regex_logline_drivesize, line).groups()[4], 16)
        elif re.match(regex_logline_lastline, line):
            blocks_accessed = int(
                re.match(regex_logline_lastline, line).groups()[4], 16)
        elif re.match(regex_logline_status, line):
            pass
        elif re.match(regex_logline_esyscall, line):
            return True
        elif re.match(regex_logline_esignal, line):
            return True
        elif re.match(regex_logline_edata, line):
            return True
        elif re.match(regex_logline_eintern, line):
            return True
        elif re.match(regex_logline_eio, line):
            return True
        elif re.match(regex_logline_eunknown, line):
            return True
        elif re.match(regex_logline_eferror, line):
            return True
        else:
            config.logger.critical('Log file format error: {0}'.format(line))
            exit(-1)
    if -1 == last_block or -1 == blocks_accessed:
        config.logger.error('Incomplete data')
        return True
    if last_block + 1 != blocks_accessed:
        config.logger.error('Invalid amount of data accessed')
        return True
    return False


def ScanLogFile(filename):
    with gzip.open(filename, 'rb') as fp:
        return ScanLog(fp)

