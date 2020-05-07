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
    ax1.set_xlabel('Time')
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
    parser.add_argument(
        '-g',
        '--sgno',
        action='store',
        type=int,
        required=True,
        help='SCSI generic device number')
    args = parser.parse_args()

    logFormatter = logging.Formatter(
        "%(asctime)s [%(levelname)-8.8s %(filename)s:%(lineno)4d]  %(message)s")
    global logger
    logger = logging.getLogger()
    fileName = '{0}-log-{1}.log'.format(
        os.path.splitext(
            os.path.basename(
                sys.argv[0]))[0], re.sub(
            ' ', '_', time.strftime(
                "%a %b %d %H:%M:%S UTC %Y", time.gmtime())))
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
                    start = parse(re.match(logscan.regex_logline_drivesize,
                                           line).groups()[0])
                    tstamp = start
                    data[filename]['x'].append(tstamp)
                    data[filename]['y'].append(0)
                elif re.match(logscan.regex_logline_lastline, line):
                    blocks_accessed = int(
                        re.match(
                            logscan.regex_logline_lastline,
                            line).groups()[4],
                        16)
                    end = parse(re.match(logscan.regex_logline_lastline,
                                         line).groups()[0])
                    data[filename]['x'].append(end)
                    data[filename]['y'].append(0)
                elif re.match(logscan.regex_logline_status, line):
                    tstamp1 = tstamp
                    blka1 = blocks_accessed
                    tstamp = parse(re.match(logscan.regex_logline_status,
                                            line).groups()[0])
                    blocks_accessed = int(
                        re.match(
                            logscan.regex_logline_status,
                            line).groups()[4],
                        16)
                    mbps = (blocks_accessed - blka1) / \
                        (2 * 1024 * (tstamp -
                                     tstamp1).total_seconds())
                    if None is last_mbps or (
                            abs(mbps - last_mbps) / last_mbps) > 0.01:
                        data[filename]['x'].append(tstamp)
                        data[filename]['y'].append(mbps)
                        last_mbps = mbps
                elif re.match(logscan.regex_logline_eferror, line) or \
                        re.match(logscan.regex_logline_eunknown, line):
                    siod_error = True
                elif re.match(logscan.regex_logline_esyscall, line):
                    siod_error = True
                elif re.match(logscan.regex_logline_edata, line):
                    cdb = re.match(logscan.regex_logline_edata,
                                   line).groups()[5]
                    cdb = ['{0:02X}'.format(int(code, 16))
                           for code in re.split(r'\s+', cdb)]
                    for i in range(int(len(cdb) / 2)):
                        cdb[i] = cdb[i * 2] + cdb[i * 2 + 1]
                    logger.debug('\tData Mismatch {0} : {1}'.format(
                        filename, ScanCDB.InterpretCDB(cdb)))
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
                        sense_code = [''.join(sense_code[i:i + 2])
                                      for i in range(0, len(sense_code), 2)]
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
                    logger.critical(
                        'Unable to parse SIOD output: {0}'.format(line))
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
                    logger.error(
                        '\t\tDrive Errors : {0} '.format(drive_errors))
                if 0 < len(host_errors):
                    for herr in host_errors:
                        logger.error('\t\t{0} : {1} '.format(
                            herr, host_errors[herr]))
                if 0 < len(device_errors):
                    for derr in device_errors:
                        logger.error('\t\t{0} : {1} '.format(
                            derr, device_errors[derr]))
                if 0 < len(sense_errors):
                    for a in sense_errors:
                        logger.error('\t\tSense Key 0x{0:02X} '.format(a))
                        for b in sense_errors[a]:
                            logger.error(
                                '\t\t\tAdditional Sense Code 0x{0:02X} '.format(b))
                            for c in sense_errors[a][b]:
                                logger.error(
                                    '\t\t\t\tAdditional Sense Code Qualifier 0x{0:02X} : {1}'.format(
                                        c, len(
                                            sense_errors[a][b][c])))
                                for d in sense_errors[a][b][c]:
                                    logger.debug(
                                        '\t\t\t\t\t{0}'.format(d['cdb']))
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
