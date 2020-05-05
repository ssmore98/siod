#!/usr/bin/python3

import time
import traceback
import ScanCDB
import ScanSenseData
import statistics
import logscan
import gzip
import tarfile
import matplotlib.dates
import datetime
import matplotlib.pyplot as pyplot
import matplotlib
from dateutil.parser import parse
from pprint import pprint
import xml.dom.minidom
import dateutil
import re
import io
import logging
import pickle
import argparse
import sys
import os
import glob

matplotlib.use('Agg')

logger = None


def getText(nodelist):
    rc = []
    for node in nodelist:
        if node.nodeType == node.TEXT_NODE:
            rc.append(node.data)
    return ''.join(rc)


def plot(plotdata, param, model):
    logger.info('Plotting {0}'.format(param))
    pyplot.figure()
    fig, ax1 = pyplot.subplots()
    ax1.set_xlabel('Day of the year')
    ax1.set_title("{0}: {1}".format(param, model))
    ax1.set_ylabel(param)
    xmin = None
    xmax = None
    ymin = None
    ymax = None
    for sno in plotdata:
        if 0 == len(plotdata[sno]['x']):
            pass
        elif 1 == len(plotdata[sno]['x']):
            pass
            ax1.plot_date(plotdata[sno]['x'], plotdata[sno]['y'], '*')
        else:
            ax1.plot_date(plotdata[sno]['x'], plotdata[sno]['y'], '-')
        if 0 < len(
            plotdata[sno]['x']) and (
            not xmin or xmin > min(
                plotdata[sno]['x'])):
            xmin = min(plotdata[sno]['x'])
        if 0 < len(
            plotdata[sno]['x']) and (
            not xmax or xmax < max(
                plotdata[sno]['x'])):
            xmax = max(plotdata[sno]['x'])
        if 0 < len(
            plotdata[sno]['y']) and (
            not ymin or ymin > min(
                plotdata[sno]['y'])):
            ymin = min(plotdata[sno]['y'])
        if 0 < len(
            plotdata[sno]['y']) and (
            not ymax or ymax < max(
                plotdata[sno]['y'])):
            ymax = max(plotdata[sno]['y'])
    if xmin and xmax and xmin == xmax:
        xmin = xmin - datetime.timedelta(1)
        xmax = xmax + datetime.timedelta(1)
    day = matplotlib.dates.DayLocator()
    month = matplotlib.dates.MonthLocator()
    dayFmt = matplotlib.dates.DateFormatter('%j')
    if xmin:
        ax1.set_xlim(xmin=xmin)
    if xmax:
        ax1.set_xlim(xmax=xmax)
    ax1.xaxis.set_major_locator(month)
    ax1.xaxis.set_major_formatter(dayFmt)
    ax1.xaxis.set_minor_locator(day)
    ax1.set_ylim(ymin=0)
    if param in ['TB Read', 'TB Written', 'Drive MBPS']:
        ax1.yaxis.set_major_formatter(
            matplotlib.ticker.FormatStrFormatter('%0.2f'))
        ax1.yaxis.set_major_locator(matplotlib.ticker.LinearLocator(8))
    else:
        ax1.yaxis.set_major_formatter(
            matplotlib.ticker.FormatStrFormatter('%d'))
        if param in ['Drive Temperature']:
            ax1.yaxis.set_major_locator(matplotlib.ticker.MultipleLocator(5))
        else:
            # ax1.yaxis.set_major_locator(matplotlib.ticker.MultipleLocator(1))
            pass
        ax1.set_ylim(ymax=ymax + 1)
    x = []
    y = []
    colors = {}
    colors['R'] = 'blue'
    colors['W'] = 'red'
    pyplot.savefig('{0}-{1}.png'.format(model, param))
    pyplot.close()


def main():
    parser = argparse.ArgumentParser(
        description="This script plots the simple I/O driver results.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="")
    parser.add_argument('-v', '--verbose', action='count',
                        help='Increase verbosity')
    parser.add_argument('-p', '--prefix', action='store', required=False,
                        help='Log file prefix')
    parser.add_argument('-g', '--sgno', action='store', type=int, required=True,
                        help='SCSI generic device number')
    args = parser.parse_args()

    logFormatter = logging.Formatter("%(asctime)s [%(levelname)-8.8s %(filename)s:%(lineno)4d]  %(message)s")
    global logger
    logger = logging.getLogger()
    fileName = '{0}-log-{1}.log'.format(os.path.splitext(os.path.basename(sys.argv[0]))[0],
            re.sub(' ', '_', time.strftime("%a %b %d %H:%M:%S UTC %Y", time.gmtime())))
    fileHandler = logging.FileHandler(fileName)
    fileHandler.setFormatter(logFormatter)
    logger.addHandler(fileHandler)
    consoleHandler = logging.StreamHandler()
    consoleHandler.setFormatter(logFormatter)
    logger.addHandler(consoleHandler)
    temp = logger.getEffectiveLevel()
    logger.setLevel(logging.INFO)
    logger.info('Command line: {0}'.format(' '.join(sys.argv)))

    if args.verbose:
        logger.setLevel(logging.DEBUG)

    if args.prefix:
        gfp = '{1}*.{0}.gz'.format(args.sgno, args.prefix)
    else:
        gfp = '*.{0}.gz'.format(args.sgno)

    data = {}
    for filename in glob.glob(gfp):
        data[filename] = {}
        data[filename]['x'] = []
        data[filename]['y'] = []
        logger.info('processing {0}'.format(filename))
        drive_mbps = {}
        total_blocks_accessed = 0
        with gzip.GzipFile(filename, 'r',) as fp:
            start = None
            end = None
            blocks_accessed = 0
            siod_error = False
            last_mbps = None
            drive_errors = 0
            sense_errors = {}
            host_errors = {}
            device_errors = {}
            for fline in fp:
                line = fline.decode().strip()
                logger.debug('\t\t{0}'.format(line))
                if re.match(logscan.RegexLoglineCommand(), line):
                    pass
                elif re.match(logscan.regex_logline_drivesize, line):
                    start = parse(re.match(logscan.regex_logline_drivesize, line).groups()[0])
                    tstamp = start
                    data[filename]['x'].append(tstamp)
                    data[filename]['y'].append(0)
                elif re.match(logscan.regex_logline_lastline, line):
                    blocks_accessed = int(re.match(logscan.regex_logline_lastline,
                        line).groups()[4], 16)
                    end = parse( re.match( logscan.regex_logline_lastline,
                        line).groups()[0])
                    data[filename]['x'].append(end)
                    data[filename]['y'].append(0)
                elif re.match(logscan.regex_logline_status, line):
                    tstamp1 = tstamp
                    blka1 = blocks_accessed
                    tstamp = parse(
                        re.match(logscan.regex_logline_status, line).groups()[0])
                    blocks_accessed = int(re.match(logscan.regex_logline_status,
                        line).groups()[4], 16)
                    mbps = (blocks_accessed - blka1) / \
                        (2 * 1024 * (tstamp -
                                     tstamp1).total_seconds())
                    if None is last_mbps or (
                            abs(mbps - last_mbps) / last_mbps) > 0.01:
                        data[filename]['x'].append(
                            tstamp)
                        data[filename]['y'].append(mbps)
                        last_mbps = mbps
                elif re.match(logscan.regex_logline_eferror, line) or \
                        re.match(logscan.regex_logline_eunknown, line):
                    siod_error = True
                elif re.match(logscan.regex_logline_esyscall, line):
                    siod_error = True
                elif re.match(logscan.regex_logline_edata, line):
                    cdb = re.match(logscan.regex_logline_edata, line).groups()[5]
                    cdb = ['{0:02X}'.format(int(code, 16)) for code in re.split(r'\s+', cdb)]
                    for i in range(int(len(cdb) / 2)):
                        cdb[i] = cdb[i * 2] + cdb[i * 2 + 1]
                    logger.debug('\tData Mismatch {0} : {1}'.format(filename, ScanCDB.InterpretCDB(cdb)))
                elif re.match(logscan.regex_logline_dmism, line):
                    drive_errors += 1
                elif re.match(logscan.regex_logline_eio, line):
                    step_error_drives.add(sno)
                    drive_errors += 1
                    # print line
                    # print
                    # re.match(logscan.regex_logline_eio,
                    # line).groups()
                    message = re.match(
                        logscan.regex_logline_eio, line).groups()[5]
                    if message in [
                        'HOST THE LOW LEVEL DRIVER WANTS A RETRY',
                        'HOST TIMEOUT',
                        'HOST RESET',
                        'HOST UNKNOWN STATUS (0x0e',
                            'HOST UNABLE TO CONNECT BEFORE TIMEOUT']:
                        if message not in host_errors:
                            host_errors[message] = 0
                        host_errors[message] += 1
                        cdb = re.match(
                            logscan.regex_logline_eio, line).groups()[7]
                    elif message in ['Check Condition', 'DRIVER SENSE ERROR']:
                        sense_data = {}
                        sense_code = re.match(
                            logscan.regex_logline_eio, line).groups()[9]
                        sense_code = ['{0:02X}'.format(int(code, 16)) for code in re.split(
                            r'\s+', sense_code.strip())]
                        sense_code = [
                            ''.join(sense_code[i:i + 2]) for i in range(0, len(sense_code), 2)]
                        OP_FIELD, K_FIELD, C_FIELD, Q_FIELD = ScanSenseData.ScanSenseData(
                            sense_code)
                        # print ScanSenseData.KeyDescription(K_FIELD)
                        # ScanSenseData.SenseDescription(K_FIELD, C_FIELD, Q_FIELD, sense_code, False)
                        sense_data['sense'] = 'TBD'
                        cdb = re.match(
                            logscan.regex_logline_eio, line).groups()[7]
                        cdb = ['{0:02X}'.format(
                            int(code, 16)) for code in re.split(r'\s+', cdb)]
                        for i in range(
                                int(len(cdb) / 2)):
                            cdb[i] = cdb[i * 2] + \
                                cdb[i * 2 + 1]
                        sense_data['cdb'] = ScanCDB.InterpretCDB(
                            cdb)
                        if K_FIELD not in sense_errors:
                            sense_errors[K_FIELD] = {
                            }
                        if C_FIELD not in sense_errors[K_FIELD]:
                            sense_errors[K_FIELD][C_FIELD] = {
                            }
                        if Q_FIELD not in sense_errors[K_FIELD][C_FIELD]:
                            sense_errors[K_FIELD][C_FIELD][Q_FIELD] = [
                            ]
                        sense_errors[K_FIELD][C_FIELD][Q_FIELD].append(
                            sense_data)
                    elif message in ['write']:
                        if message not in host_errors:
                            device_errors[message] = [
                            ]
                        device_errors[message].append(
                            re.match(logscan.regex_logline_eio, line).groups()[6])
                    else:
                        config.logger.critical(
                            'Cannot parse: {0}'.format(line))
                        exit(-1)
                elif re.match(logscan.regex_logline_esignal, line):
                    pass
                else:
                    logger.critical('Unable to parse SIOD output: {0}'.format(line))
                    exit(-1)
            if None is not start and None is not end and False == siod_error:
                drive_mbps[filename] = float(blocks_accessed) / \
                    ((2 * 1000) *
                     (end - start).total_seconds())
                total_blocks_accessed += blocks_accessed
                if 0 < drive_errors or 0 < len(
                        host_errors) or 0 < len(device_errors):
                    logger.error('\t{0} Errors:'.format(filename))
                if 0 < drive_errors:
                    logger.error('\t\tDrive Errors : {0} '.format(drive_errors))
                if 0 < len(host_errors):
                    for herr in host_errors:
                        logger.error('\t\t{0} : {1} '.format(herr, host_errors[herr]))
                if 0 < len(device_errors):
                    for derr in device_errors:
                        logger.error('\t\t{0} : {1} '.format(derr, device_errors[derr]))
                if 0 < len(sense_errors):
                    for a in sense_errors:
                        logger.error('\t\tSense Key 0x{0:02X} '.format(a))
                        for b in sense_errors[a]:
                            logger.error('\t\t\tAdditional Sense Code 0x{0:02X} '.format(b))
                            for c in sense_errors[a][b]:
                                logger.error(
                                        '\t\t\t\tAdditional Sense Code Qualifier 0x{0:02X} : {1}'.format(c,
                                            len(sense_errors[a][b][c])))
                                for d in sense_errors[a][b][c]:
                                    logger.debug('\t\t\t\t\t{0}'.format(d['cdb']))
            logger.info('\tData         = {0}TB'.format(
                total_blocks_accessed / (2 * 1000 * 1000 * 1000)))
            if 0 < drive_errors:
                logger.error('\tErrors       = {0}'.format(drive_errors))
            logger.info('\tGBPS         = {0:0.2f}'.format(float(
                total_blocks_accessed / (2 * 1000 * 1000) / (end - start).total_seconds())))
            logger.info('\tDrive Time   = {0}'.format(end - start))
            logger.info('\tDrive MBPS   = {0:.2f}/{1:.2f}/{2:.2f}'.format(
                statistics.mean([drive_mbps[sno] for sno in drive_mbps]),
                statistics.median([drive_mbps[sno] for sno in drive_mbps]),
                statistics.pstdev([drive_mbps[sno] for sno in drive_mbps])))

            plot(data, "Drive_MBPS", filename)
#
#    data = {}
#    if 'logdata' in cache:
#        data = cache['logdata']
#    tstamps = set()
#    if 'tstamps' in cache:
#        tstamps = cache['tstamps']
#    is_sata = set()
#    if 'is_sata' in cache:
#        is_sata = cache['is_sata']
#    update = False
#    for tstamp in logdata.keys():
#        tstamp1 = parse(tstamp)
#        if tstamp1 in tstamps:
#            continue
#        update = True
#        tstamps.add(tstamp1)
#        if isinstance(logdata[tstamp], dict):
#            drivelogs = logdata[tstamp]
#        elif isinstance(logdata[tstamp], str):
#            if os.path.exists(logdata[tstamp]):
#                if os.path.isfile(logdata[tstamp]):
#                    if os.access(logdata[tstamp], os.R_OK):
#                        config.logger.debug(
#                            'Reading drive logs from {0}'.format(
#                                logdata[tstamp]))
#                        with open(logdata[tstamp], 'rb') as fp:
#                            drivelogs = pickle.load(fp)
#                    else:
#                        config.logger.error(
#                            'Cannot read drive logs at {1}: {0} is not readable'.format(
#                                logdata[tstamp], tstamp))
#                        continue
#                else:
#                    config.logger.error(
#                        'Cannot read drive logs at {1}: {0} is not a file'.format(
#                            logdata[tstamp], tstamp))
#                    continue
#            else:
#                config.logger.error(
#                    'Cannot read drive logs at {1}: {0} does not exist'.format(
#                        logdata[tstamp], tstamp))
#                continue
#        else:
#            config.logger.error('Cannot read drive logs at {0}'.format(tstamp))
#            continue
#        for node in drivelogs.keys():
#            config.logger.debug('Processing node {0}'.format(node))
#            for key in drivelogs[node].keys():
#                config.logger.debug('\tkey={0}'.format(key))
#                if isinstance(drivelogs[node][key], dict):
#                    smart_counter = int(key)
#                    for sno in drivelogs[node][key]:
#                        if sno not in drives[node]:
#                            config.logger.debug(
#                                'Drive {0} not found in the drive list.'.format(sno))
#                            continue
#                        config.logger.debug('\t\tsno={0}'.format(sno))
#                        is_sata.add(sno)
#                        model = drives[node][sno]['model']
#                        if sno not in data:
#                            data[sno] = {}
#                        if tstamp1 not in data[sno]:
#                            data[sno][tstamp1] = {}
#                        if model in [
#                            'MZ7LM3T8HMLP-00005',
#                            'MZ7LM1T9HMJP-00005',
#                                'MZ7LH1T9HMLT-00005']:
#                            if 0xF2 == smart_counter:
#                                data[sno][tstamp1]['blocks_read'] = drivelogs[node][key][sno][1]
#                            elif 0xF1 == smart_counter:
#                                data[sno][tstamp1]['blocks_written'] = drivelogs[node][key][sno][1]
#                            elif 0xBB == smart_counter:
#                                data[sno][tstamp1]['uncorrected_errors'] = drivelogs[node][key][sno][1]
#                            elif 0xC2 == smart_counter:
#                                data[sno][tstamp1]['temp'] = drivelogs[node][key][sno][1]
#                        elif model in ['MTFDDAK960MAV', 'MTFDDAK960TBY']:
#                            if 0xF6 == smart_counter:
#                                data[sno][tstamp1]['blocks_written'] = drivelogs[node][key][sno][1]
#                            elif 0xBB == smart_counter:
#                                data[sno][tstamp1]['uncorrected_errors'] = drivelogs[node][key][sno][1]
#                            elif 0xC2 == smart_counter:
#                                data[sno][tstamp1]['temp'] = drivelogs[node][key][sno][1]
#                        elif model in ['SSDSC2KB019T7', 'SSDSC2BB016T4', 'SSDSC2KB019T8']:
#                            if 0xF2 == smart_counter:
#                                data[sno][tstamp1]['blocks_read'] = drivelogs[node][key][sno][1]
#                            elif 0xF1 == smart_counter:
#                                data[sno][tstamp1]['blocks_written'] = drivelogs[node][key][sno][1]
#                            elif 0xBB == smart_counter:
#                                data[sno][tstamp1]['uncorrected_errors'] = drivelogs[node][key][sno][1]
#                            elif 0xC2 == smart_counter:
#                                data[sno][tstamp1]['temp'] = drivelogs[node][key][sno][1]
#                        else:
#                            config.logger.error(
#                                'Drive {0}: unknown model {1}'.format(
#                                    sno, model))
#                            del data[sno]
#                            continue
#                else:
#                    sno = key
#                    if sno not in drives[node]:
#                        config.logger.debug(
#                            'Drive {0} not found in the drive list.'.format(sno))
#                        continue
#                    counters = {}
#                    counters['read'] = {}
#                    counters['write'] = {}
#                    counters['verify'] = {}
#                    xfd = xml.dom.minidom.parseString(drivelogs[node][sno])
#                    root = xfd.documentElement
#                    for page in root.getElementsByTagName('page'):
#                        pageno = int(page.getAttribute('number'))
#                        lines = [l for l in [l.strip() for l in getText(
#                            page.childNodes).split('\n')] if 0 < len(l)]
#                        if pageno in (0x2, 0x3, 0x5):
#                            error_wo_delay = 0
#                            for line in lines:
#                                title = r'Write error counter page|Read error counter page|Verify error counter page'
#                                pattern1 = r'Errors corrected with possible delays = (\d+)'
#                                pattern2 = r'Total rewrites or rereads = (\d+)'
#                                pattern3 = r'Total errors corrected = (\d+)'
#                                pattern4 = r'Total times correction algorithm processed = (\d+)'
#                                pattern5 = r'Total bytes processed = (\d+)'
#                                pattern6 = r'Total uncorrected errors = (\d+)'
#                                pattern7 = r'Errors corrected without substantial delay = (\d+)'
#                                pattern8 = r'Reserved or vendor specific \[0x[a-f\d]+\] = (\d+)'
#                                if re.match(title, line):
#                                    pass
#                                elif re.match(pattern1, line):
#                                    error_w_delay = int(
#                                        re.match(pattern1, line).groups()[0])
#                                elif re.match(pattern2, line):
#                                    pass
#                                elif re.match(pattern3, line):
#                                    corrected_errors = int(
#                                        re.match(pattern3, line).groups()[0])
#                                elif re.match(pattern4, line):
#                                    pass
#                                elif re.match(pattern5, line):
#                                    if drives[node][sno]['vendor'].upper() in [
#                                            'TOSHIBA']:
#                                        if 0x3 == pageno:
#                                            counters['blocks_read'] = int(
#                                                int(re.match(pattern5, line).groups()[0]) / 512)
#                                        elif 0x2 == pageno:
#                                            counters['blocks_written'] = int(
#                                                int(re.match(pattern5, line).groups()[0]) / 512)
#                                elif re.match(pattern6, line):
#                                    uncorrected_errors = int(
#                                        re.match(pattern6, line).groups()[0])
#                                elif re.match(pattern7, line):
#                                    error_wo_delay = int(
#                                        re.match(pattern7, line).groups()[0])
#                                elif re.match(pattern8, line):
#                                    pass
#                                else:
#                                    config.logger.error(
#                                        'Cannot parse line in log page 0x{0:X}: {1}'.format(
#                                            pageno, line))
#                            if 0x2 == pageno:
#                                counter = counters['write']
#                            elif 0x3 == pageno:
#                                counter = counters['read']
#                            elif 0x5 == pageno:
#                                counter = counters['verify']
#                            counter['error_w_delay'] = error_w_delay
#                            counter['error_wo_delay'] = error_wo_delay
#                            counter['corrected_errors'] = corrected_errors
#                            counter['uncorrected_errors'] = uncorrected_errors
#                        elif pageno == 0x6:
#                            for line in lines:
#                                title = r'Non-medium error page'
#                                pattern1 = r'Non-medium error count = (\d+)'
#                                if re.match(title, line):
#                                    pass
#                                elif re.match(pattern1, line):
#                                    nm_errors = int(
#                                        re.match(pattern1, line).groups()[0])
#                                else:
#                                    config.logger.error(
#                                        'Cannot parse line in log page 0x{0:X}: {1}'.format(
#                                            pageno, line))
#                            counters['nm_errors'] = nm_errors
#                        elif pageno == 0xd:
#                            for line in lines:
#                                title = r'(Temperature log page|{0})'.format(
#                                    re.escape('Temperature page  (spc-3) [0xd]'))
#                                pattern1 = r'Current temperature = (\d+) C'
#                                pattern2 = r'Reference temperature = (\d+) C'
#                                if re.match(title, line):
#                                    pass
#                                elif re.match(pattern1, line):
#                                    temp = int(
#                                        re.match(pattern1, line).groups()[0])
#                                elif re.match(pattern2, line):
#                                    ref_temp = int(
#                                        re.match(pattern2, line).groups()[0])
#                                else:
#                                    config.logger.error(
#                                        'Cannot parse line in log page 0x{0:X}: {1}'.format(
#                                            pageno, line))
#                            counters['temp'] = temp
#                            counters['ref_temp'] = ref_temp
#                        elif pageno == 0x3e:
#                            for line in lines:
#                                title = re.escape(
#                                    'Seagate/Hitachi factory page [0x3e]')
#                                pattern1 = r'number of hours powered up = (\d+\.\d+)'
#                                pattern2 = r'number of minutes until next internal SMART test = (\d+)'
#                                pattern3 = r'Unknown Seagate/Hitachi parameter code = 0x([a-fA-F\d]+)'
#                                if re.match(title, line):
#                                    pass
#                                elif re.match(pattern1, line):
#                                    poweron_hours = float(
#                                        re.match(pattern1, line).groups()[0])
#                                elif re.match(pattern2, line):
#                                    pass
#                                elif re.match(pattern3, line):
#                                    pass
#                                else:
#                                    config.logger.error(
#                                        'Cannot parse line in log page 0x{0:X}: {1}'.format(
#                                            pageno, line))
#                            counters['poweron_hours'] = poweron_hours
#                        elif pageno == 0x2f:
#                            for line in lines:
#                                title = r'(Informational Exceptions log page|{0})'.format(
#                                    re.escape('Informational Exceptions page  (spc-3) [0x2f]'))
#                                pattern1 = r'Current temperature = (\d+) C'
#                                pattern2 = r'Threshold temperature = (\d+) C'
#                                hx = r'0x([a-fA-F\d]+)'
#                                pattern3 = r'IE asc = {0}, ascq = {0}'.format(
#                                    hx)
#                                pattern4 = r'\[ASC={0}, ASCQ={0}( \(hex\))?\]'.format(
#                                    '[ a-fA-F\d]{2}')
#                                pattern5 = r'\[Additional sense: (.+)\]'
#                                if re.match(title, line):
#                                    pass
#                                elif re.match(pattern1, line):
#                                    pass
#                                elif re.match(pattern2, line):
#                                    pass
#                                elif re.match(pattern3, line):
#                                    asc = int(
#                                        re.match(
#                                            pattern3,
#                                            line).groups()[0],
#                                        16)
#                                    ascq = int(
#                                        re.match(
#                                            pattern3,
#                                            line).groups()[1],
#                                        16)
#                                elif re.match(pattern4, line):
#                                    pass
#                                elif re.match(pattern5, line):
#                                    pass
#                                else:
#                                    config.logger.error(
#                                        'Cannot parse line in log page 0x{0:X}: {1}'.format(
#                                            pageno, line))
#                            counters['asc'] = asc
#                            counters['ascq'] = ascq
#                        elif pageno == 0x19:
#                            if drives[node][sno]['vendor'].upper() in ['HGST']:
#                                block_read = 0
#                                blocks_written = 0
#                                title = re.escape('log_page=0x19')
#                                p1 = r'parameter_code=\d+'
#                                p2 = r'read_commands=\d+'
#                                p3 = r'write_commands=\d+'
#                                p4 = r'lb_received=(\d+)'
#                                p5 = r'lb_transmitted=(\d+)'
#                                p6 = r'read_proc_intervals=\d+'
#                                p7 = r'write_proc_intervals=\d+'
#                                p8 = r'weight_rw_commands=\d+'
#                                p9 = r'weight_rw_processing=\d+'
#                                p10 = r'parameter_code=\d+'
#                                p11 = r'idle_time_intervals=\d+'
#                                p12 = r'parameter_code=\d+'
#                                p13 = r'time_interval_neg_exp=\d+'
#                                p14 = r'time_interval_int=\d+'
#                                for line in lines:
#                                    if re.match(
#                                            title, line) or re.match(
#                                            p1, line) or re.match(
#                                            p2, line) or re.match(
#                                            p3, line):
#                                        pass
#                                    elif re.match(p6, line) or re.match(p7, line) or re.match(p8, line) or re.match(p9, line):
#                                        pass
#                                    elif re.match(p10, line) or re.match(p11, line) or re.match(p12, line) or re.match(p13, line):
#                                        pass
#                                    elif re.match(p14, line):
#                                        pass
#                                    elif re.match(p5, line):
#                                        blocks_written = int(
#                                            re.match(p5, line).groups()[0])
#                                    elif re.match(p4, line):
#                                        blocks_read = int(
#                                            re.match(p4, line).groups()[0])
#                                    else:
#                                        config.logger.error(
#                                            'Cannot parse line in log page 0x{0:X}: {1}'.format(
#                                                pageno, line))
#                                counters['blocks_read'] = blocks_read
#                                counters['blocks_written'] = blocks_written
#                        elif pageno == 0x37:
#                            if drives[node][sno]['vendor'].upper() in [
#                                    'SEAGATE']:
#                                block_read = 0
#                                blocks_written = 0
#                                title = re.escape('Seagate cache page [0x37]')
#                                pattern1 = r'Blocks sent to initiator = (\d+)'
#                                pattern2 = r'Blocks received from initiator = (\d+)'
#                                pattern3 = r'Blocks read from cache and sent to initiator = (\d+)'
#                                pattern4 = r'{0} = (\d+)'.format(
#                                    re.escape('Number of read and write commands whose size <= segment size'))
#                                pattern5 = r'{0} = (\d+)'.format(
#                                    re.escape('Number of read and write commands whose size > segment size'))
#                                for line in lines:
#                                    if re.match(title, line):
#                                        pass
#                                    elif re.match(pattern1, line):
#                                        blocks_read = int(
#                                            re.match(pattern1, line).groups()[0])
#                                    elif re.match(pattern2, line):
#                                        blocks_written = int(
#                                            re.match(pattern2, line).groups()[0])
#                                    elif re.match(pattern3, line):
#                                        pass
#                                    elif re.match(pattern4, line):
#                                        pass
#                                    elif re.match(pattern5, line):
#                                        pass
#                                    else:
#                                        config.logger.error(
#                                            'Cannot parse line in log page 0x{0:X}: {1}'.format(
#                                                pageno, line))
#                                counters['blocks_read'] = blocks_read
#                                counters['blocks_written'] = blocks_written
#                    if sno in data:
#                        if tstamp1 in data[sno]:
#                            pass
#                        else:
#                            data[sno][tstamp1] = counters
#                    else:
#                        data[sno] = {}
#                        data[sno][tstamp1] = counters
#    for sno in data:
#        blocks_read = int(0)
#        blocks_written = int(0)
#        l_blocks_read = None
#        l_blocks_written = None
#        nm_errors = int(0)
#        l_nm_errors = None
#        for tstamp in sorted(tstamps):
#            if tstamp in data[sno]:
#                if 'blocks_read' in data[sno][tstamp]:
#                    if None is not l_blocks_read and l_blocks_read > data[sno][tstamp]['blocks_read']:
#                        blocks_read += 4294967296
#                    l_blocks_read = data[sno][tstamp]['blocks_read']
#                    data[sno][tstamp]['blocks_read'] += blocks_read
#
#                if 'blocks_written' in data[sno][tstamp]:
#                    if None is not l_blocks_written and l_blocks_written > data[
#                            sno][tstamp]['blocks_written']:
#                        blocks_written += 4294967296
#                    l_blocks_written = data[sno][tstamp]['blocks_written']
#                    data[sno][tstamp]['blocks_written'] += blocks_written
#
#                if 'nm_errors' in data[sno][tstamp]:
#                    if None is not l_nm_errors and l_nm_errors > data[sno][tstamp]['nm_errors']:
#                        nm_errors += 4294967296
#                    l_nm_errors = data[sno][tstamp]['nm_errors']
#                    data[sno][tstamp]['nm_errors'] += nm_errors
#        for op in ('read', 'write', 'verify'):
#            error_w_delay = int(0)
#            l_error_w_delay = None
#            error_wo_delay = int(0)
#            l_error_wo_delay = None
#            corrected_errors = int(0)
#            l_corrected_errors = None
#            uncorrected_errors = int(0)
#            l_uncorrected_errors = None
#            for tstamp in sorted(tstamps):
#                if tstamp in data[sno] and op in data[sno][tstamp]:
#                    if 'error_w_delay' in data[sno][tstamp][op].keys():
#                        if None is not l_error_w_delay and l_error_w_delay > data[
#                                sno][tstamp][op]['error_w_delay']:
#                            error_w_delay += 4294967296
#                        l_error_w_delay = data[sno][tstamp][op]['error_w_delay']
#                        data[sno][tstamp][op]['error_w_delay'] += error_w_delay
#
#                    if 'error_wo_delay' in data[sno][tstamp][op].keys():
#                        if None is not l_error_wo_delay and l_error_wo_delay > data[
#                                sno][tstamp][op]['error_wo_delay']:
#                            error_wo_delay += 4294967296
#                        l_error_wo_delay = data[sno][tstamp][op]['error_wo_delay']
#                        data[sno][tstamp][op]['error_wo_delay'] += error_wo_delay
#
#                    if 'corrected_errors' in data[sno][tstamp][op].keys():
#                        if None is not l_corrected_errors and l_corrected_errors > data[
#                                sno][tstamp][op]['corrected_errors']:
#                            corrected_errors += 4294967296
#                        l_corrected_errors = data[sno][tstamp][op]['corrected_errors']
#                        data[sno][tstamp][op]['corrected_errors'] += corrected_errors
#
#                    if 'uncorrected_errors' in data[sno][tstamp][op].keys():
#                        if None is not l_uncorrected_errors and l_uncorrected_errors > data[
#                                sno][tstamp][op]['uncorrected_errors']:
#                            uncorrected_errors += 4294967296
#                        l_uncorrected_errors = data[sno][tstamp][op]['uncorrected_errors']
#                        data[sno][tstamp][op]['uncorrected_errors'] += uncorrected_errors
#    if update:
#        config.logger.info(
#            'Writing the cached information to {0}'.format(datafilename))
#        with open(datafilename, 'wb') as fp:
#            cache['logdata'] = data
#            cache['tstamps'] = tstamps
#            cache['is_sata'] = is_sata
#            # pickle.dump(cache, fp)
#
#    plotdata = {}
#    for sno in data:
#        xdata = []
#        ydata = []
#        for tstamp in sorted(tstamps):
#            if tstamp in data[sno]:
#                if 'temp' in data[sno][tstamp]:
#                    xdata.append(tstamp)
#                    ydata.append(data[sno][tstamp]['temp'])
#        if 0 < len(xdata) and 0 < len(ydata):
#            plotdata[sno] = {}
#            plotdata[sno]['x'] = xdata
#            plotdata[sno]['y'] = ydata
#    if len(plotdata):
#        plot(plotdata, "Drive Temperature",
#             stateinfo['system'], stateinfo['models'], steps)
#
#    plotdata = {}
#    for sno in data:
#        config.logger.debug('{0}: uncorrected errors'.format(sno))
#        xdata = []
#        ydata = []
#        base = None
#        for tstamp in sorted(tstamps):
#            if sno in is_sata:
#                if tstamp in data[sno] and 'uncorrected_errors' in data[sno][tstamp]:
#                    xdata.append(tstamp)
#                    y = data[sno][tstamp]['uncorrected_errors']
#                    if None is base:
#                        base = y
#                    ydata.append(y - base)
#            else:
#                if tstamp in data[sno]:
#                    xdata.append(tstamp)
#                    y = sum([data[sno][tstamp][op]['uncorrected_errors'] for op in (
#                        'read', 'write', 'verify') if 'uncorrected_errors' in data[sno][tstamp][op]])
#                    if None is base:
#                        base = y
#                    ydata.append(y - base)
#        if 0 < len(xdata) and 0 < len(ydata):
#            plotdata[sno] = {}
#            plotdata[sno]['x'] = xdata
#            plotdata[sno]['y'] = ydata
#    if len(plotdata):
#        plot(plotdata, "Uncorrected Errors",
#             stateinfo['system'], stateinfo['models'], steps)
#
#    plotdata = {}
#    for sno in data:
#        if sno in is_sata:
#            continue
#        xdata = []
#        ydata = []
#        base = None
#        for tstamp in sorted(tstamps):
#            if sno in is_sata:
#                if tstamp in data[sno] and 'corrected_errors' in data[sno][tstamp]:
#                    xdata.append(tstamp)
#                    y = data[sno][tstamp]['corrected_errors']
#                    if None is base:
#                        base = y
#                    ydata.append(y - base)
#            else:
#                if tstamp in data[sno]:
#                    xdata.append(tstamp)
#                    y = sum([data[sno][tstamp][op]['corrected_errors'] for op in (
#                        'read', 'write', 'verify') if 'corrected_errors' in data[sno][tstamp][op]])
#                    if None is base:
#                        base = y
#                    ydata.append(y - base)
#        if 0 < len(xdata) and 0 < len(ydata):
#            plotdata[sno] = {}
#            plotdata[sno]['x'] = xdata
#            plotdata[sno]['y'] = ydata
#    if len(plotdata):
#        plot(plotdata, "Corrected Errors",
#             stateinfo['system'], stateinfo['models'], steps)
#
#    plotdata = {}
#    for sno in data:
#        if sno in is_sata:
#            continue
#        xdata = []
#        ydata = []
#        base = None
#        for tstamp in sorted(tstamps):
#            if sno in is_sata:
#                if tstamp in data[sno] and 'error_w_delay' in data[sno][tstamp]:
#                    xdata.append(tstamp)
#                    y = data[sno][tstamp]['error_w_delay']
#                    if None is base:
#                        base = y
#                    ydata.append(y - base)
#            else:
#                if tstamp in data[sno]:
#                    xdata.append(tstamp)
#                    y = sum([data[sno][tstamp][op]['error_w_delay'] for op in (
#                        'read', 'write', 'verify') if 'error_w_delay' in data[sno][tstamp][op]])
#                    if None is base:
#                        base = y
#                    ydata.append(y - base)
#        if 0 < len(xdata) and 0 < len(ydata):
#            plotdata[sno] = {}
#            plotdata[sno]['x'] = xdata
#            plotdata[sno]['y'] = ydata
#    if len(plotdata):
#        plot(plotdata, "Errors affecting latency",
#             stateinfo['system'], stateinfo['models'], steps)
#
#    plotdata = {}
#    for sno in data:
#        if sno in is_sata:
#            continue
#        xdata = []
#        ydata = []
#        base = None
#        for tstamp in sorted(tstamps):
#            if tstamp in data[sno]:
#                if 'nm_errors' in data[sno][tstamp]:
#                    xdata.append(tstamp)
#                    y = data[sno][tstamp]['nm_errors']
#                    if None is base:
#                        base = y
#                    ydata.append(y - base)
#        if 0 < len(xdata) and 0 < len(ydata):
#            plotdata[sno] = {}
#            plotdata[sno]['x'] = xdata
#            plotdata[sno]['y'] = ydata
#    if len(plotdata):
#        plot(plotdata, "Non-medium Errors",
#             stateinfo['system'], stateinfo['models'], steps)
#
#    plotdata = {}
#    for sno in data:
#        if 'blocks_read' in frozenset(
#                [key for tstamp in data[sno] for key in data[sno][tstamp]]):
#            xdata = []
#            ydata = []
#            base = None
#            for tstamp in sorted(tstamps):
#                if tstamp in data[sno] and 'blocks_read' in data[sno][tstamp]:
#                    xdata.append(tstamp)
#                    y = data[sno][tstamp]['blocks_read']
#                    if None is base:
#                        base = y
#                    ydata.append(float(y - base) /
#                                 float(2 * 1000 * 1000 * 1000))
#            if 0 < len(xdata) and 0 < len(ydata):
#                plotdata[sno] = {}
#                plotdata[sno]['x'] = xdata
#                plotdata[sno]['y'] = ydata
#    if len(plotdata):
#        plot(plotdata, "TB Read",
#             stateinfo['system'], stateinfo['models'], steps)
#
#    plotdata = {}
#    for sno in data:
#        if 'blocks_written' in frozenset(
#                [key for tstamp in data[sno] for key in data[sno][tstamp]]):
#            xdata = []
#            ydata = []
#            base = None
#            for tstamp in sorted(tstamps):
#                if tstamp in data[sno] and 'blocks_written' in data[sno][tstamp]:
#                    xdata.append(tstamp)
#                    y = data[sno][tstamp]['blocks_written']
#                    if None is base:
#                        base = y
#                    ydata.append(float(y - base) /
#                                 float(2 * 1000 * 1000 * 1000))
#            if 0 < len(xdata) and 0 < len(ydata):
#                plotdata[sno] = {}
#                plotdata[sno]['x'] = xdata
#                plotdata[sno]['y'] = ydata
#    if len(plotdata):
#        plot(plotdata, "TB Written",
#             stateinfo['system'], stateinfo['models'], steps)
#

if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        fp = io.StringIO()
        logger.critical('Unhandled exception: {0}'.format(type(e)))
        logger.critical('Arguments: {0}'.format(e.args))
        traceback.print_exc(file=fp)
        for line in fp.getvalue().splitlines():
            logger.critical(line.strip())
        exit(-1)
