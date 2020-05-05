import re

from SeagateSenseCodes import seagate_sense_codes
from HGSTSenseCodes import hgst_sense_codes

sense_decode_data = (
    "00,00,00,No error (No Sense)",
    "00,5D,00,No sense - PFA threshold reached (No Sense)",
    "01,01,00,Recovered Write error - no index (Soft Error)",
    "01,02,00,Recovered no seek completion (Soft Error)",
    "01,03,00,Recovered Write error - write fault (Soft Error)",
    "01,09,00,Track following error (Soft Error)",
    "01,0B,01,Temperature warning (Soft Error)",
    "01,0C,01,Recovered Write error with auto-realloc - reallocated (Soft Error)",
    "01,0C,03,Recovered Write error - recommend reassign (Soft Error)",
    "01,12,01,Recovered data without ECC using prev logical block ID (Soft Error)",
    "01,12,02,Recovered data with ECC using prev logical block ID (Soft Error)",
    "01,14,01,Recovered Record Not Found (Soft Error)",
    "01,16,00,Recovered Write error - Data Sync Mark Error (Soft Error)",
    "01,16,01,Recovered Write error - Data Sync Error - data rewritten (Soft Error)",
    "01,16,02,Recovered Write error - Data Sync Error - recommend rewrite (Soft Error)",
    "01,16,03,Recovered Write error - Data Sync Error - data auto-reallocated (Soft Error)",
    "01,16,04,Recovered Write error - Data Sync Error - recommend reassignment (Soft Error)",
    "01,17,00,Recovered data with no error correction applied (Soft Error)",
    "01,17,01,Recovered Read error - with retries (Soft Error)",
    "01,17,02,Recovered data using positive offset (Soft Error)",
    "01,17,03,Recovered data using negative offset (Soft Error)",
    "01,17,05,Recovered data using previous logical block ID (Soft Error)",
    "01,17,06,Recovered Read error - without ECC, auto reallocated (Soft Error)",
    "01,17,07,Recovered Read error - without ECC, recommend reassign (Soft Error)",
    "01,17,08,Recovered Read error - without ECC, recommend rewrite (Soft Error)",
    "01,17,09,Recovered Read error - without ECC, data rewritten (Soft Error)",
    "01,18,00,Recovered Read error - with ECC (Soft Error)",
    "01,18,01,Recovered data with ECC and retries (Soft Error)",
    "01,18,02,Recovered Read error - with ECC, auto reallocated (Soft Error)",
    "01,18,05,Recovered Read error - with ECC, recommend reassign (Soft Error)",
    "01,18,06,Recovered data using ECC and offsets (Soft Error)",
    "01,18,07,Recovered Read error - with ECC, data rewritten (Soft Error)",
    "01,1C,00,Defect List not found (Soft Error)",
    "01,1C,01,Primary defect list not found (Soft Error)",
    "01,1C,02,Grown defect list not found (Soft Error)",
    "01,1F,00,Partial defect list transferred (Soft Error)",
    "01,44,00,Internal target failure (Soft Error)",
    "01,5D,00,PFA threshold reached (Soft Error)",
    "02,04,00,Not Ready - Cause not reportable. (Not Ready)",
    "02,04,01,Not Ready - becoming ready (Not Ready)",
    "02,04,02,Not Ready - need initialize command (start unit) (Not Ready)",
    "02,04,03,Not Ready - manual intervention required (Not Ready)",
    "02,04,04,Not Ready - format in progress (Not Ready)",
    "02,04,09,Not Ready - self-test in progress (Not Ready)",
    "02,04,22,Not Ready - Logical unit not ready - power cycle required.",
    "02,31,00,Not Ready - medium format corrupted (Not Ready)",
    "02,31,01,Not Ready - format command failed (Not Ready)",
    "02,35,02,Not Ready - enclosure services unavailable (Not Ready)",
    "02,3A,00,Not Ready - medium not present (Not Ready)",
    "02,3A,01,Not Ready - medium not present - tray closed (Not Ready)",
    "02,3A,02,Not Ready - medium not present - tray open (Not Ready)",
    "02,4C,00,Diagnostic Failure - config not loaded (Not Ready)",
    "03,02,00,Medium Error - No Seek Complete (Medium Error)",
    "03,03,00,Medium Error - write fault (Medium Error)",
    "03,10,00,Medium Error - ID CRC error (Medium Error)",
    "03,11,00,Medium Error - unrecovered read error (Medium Error)",
    "03,11,01,Medium Error - read retries exhausted (Medium Error)",
    "03,11,02,Medium Error - error too long to correct (Medium Error)",
    "03,11,04,Medium Error - unrecovered read error - auto re-alloc failed (Medium Error)",
    "03,11,0B,Medium Error - unrecovered read error - recommend reassign (Medium Error)",
    "03,14,01,Medium Error - record not found (Medium Error)",
    "03,16,00,Medium Error - Data Sync Mark error (Medium Error)",
    "03,16,04,Medium Error - Data Sync Error - recommend reassign (Medium Error)",
    "03,19,00,Medium Error - defect list error (Medium Error)",
    "03,19,01,Medium Error - defect list not available (Medium Error)",
    "03,19,02,Medium Error - defect list error in primary list (Medium Error)",
    "03,19,03,Medium Error - defect list error in grown list (Medium Error)",
    "03,19,0E,Medium Error - fewer than 50% defect list copies (Medium Error)",
    "03,31,00,Medium Error - medium format corrupted (Medium Error)",
    "03,31,01,Medium Error - format command failed (Medium Error)",
    "04,01,00,Hardware Error - no index or sector (Hardware Error)",
    "04,02,00,Hardware Error - no seek complete (Hardware Error)",
    "04,03,00,Hardware Error - write fault (Hardware Error)",
    "04,09,00,Hardware Error - track following error (Hardware Error)",
    "04,11,00,Hardware Error - unrecovered read error in reserved area (Hardware Error)",
    "04,15,01,Hardware Error - Mechanical positioning error (Hardware Error)",
    "04,16,00,Hardware Error - Data Sync Mark error in reserved area (Hardware Error)",
    "04,19,00,Hardware Error - defect list error (Hardware Error)",
    "04,19,02,Hardware Error - defect list error in Primary List (Hardware Error)",
    "04,19,03,Hardware Error - defect list error in Grown List (Hardware Error)",
    "04,31,00,Hardware Error - reassign failed (Hardware Error)",
    "04,32,00,Hardware Error - no defect spare available (Hardware Error)",
    "04,35,01,Hardware Error - unsupported enclosure function (Hardware Error)",
    "04,35,02,Hardware Error - enclosure services unavailable (Hardware Error)",
    "04,35,03,Hardware Error - enclosure services transfer failure (Hardware Error)",
    "04,35,04,Hardware Error - enclosure services refused (Hardware Error)",
    "04,35,05,Hardware Error - enclosure services checksum error (Hardware Error)",
    "04,3E,03,Hardware Error - self-test failed (Hardware Error)",
    "04,3E,04,Hardware Error - unable to update self-test (Hardware Error)",
    "04,44,00,Hardware Error - internal target failure (Hardware Error)",
    "05,1A,00,Illegal Request - parm list length error (Illegal Request)",
    "05,20,00,Illegal Request - invalid/unsupported command code (Illegal Request)",
    "05,21,00,Illegal Request - LBA out of range (Illegal Request)",
    "05,24,00,Illegal Request - invalid field in CDB (Command Descriptor Block) (Illegal Request)",
    "05,25,00,Illegal Request - invalid LUN (Illegal Request)",
    "05,26,00,Illegal Request - invalid fields in parm list (Illegal Request)",
    "05,26,01,Illegal Request - parameter not supported (Illegal Request)",
    "05,26,02,Illegal Request - invalid parm value (Illegal Request)",
    "05,26,03,Illegal Request - invalid field parameter - threshold parameter (Illegal Request)",
    "05,26,04,Illegal Request - invalid release of persistent reservation (Illegal Request)",
    "05,2C,00,Illegal Request - command sequence error (Illegal Request)",
    "05,35,01,Illegal Request - unsupported enclosure function (Illegal Request)",
    "05,49,00,Illegal Request - invalid message (Illegal Request)",
    "05,53,00,Illegal Request - media load or eject failed (Illegal Request)",
    "05,53,01,Illegal Request - unload tape failure (Illegal Request)",
    "05,53,02,Illegal Request - medium removal prevented (Illegal Request)",
    "05,55,00,Illegal Request - system resource failure (Illegal Request)",
    "05,55,01,Illegal Request - system buffer full (Illegal Request)",
    "05,55,04,Illegal Request - Insufficient Registration Resources (Illegal Request)",
    "06,28,00,Unit Attention - not-ready to ready transition (format complete) (Unit Attention)",
    "06,29,00,Unit Attention - POR or device reset occurred (Unit Attention)",
    "06,29,01,Unit Attention - POR occurred (Unit Attention)",
    "06,29,02,Unit Attention - SCSI bus reset occurred (Unit Attention)",
    "06,29,03,Unit Attention - TARGET RESET occurred (Unit Attention)",
    "06,29,04,Unit Attention - self-initiated-reset occurred (Unit Attention)",
    "06,29,05,Unit Attention - transceiver mode change to SE (Unit Attention)",
    "06,29,06,Unit Attention - transceiver mode change to LVD (Unit Attention)",
    "06,2A,00,Unit Attention - parameters changed (Unit Attention)",
    "06,2A,01,Unit Attention - mode parameters changed (Unit Attention)",
    "06,2A,02,Unit Attention - log select parms changed (Unit Attention)",
    "06,2A,03,Unit Attention - Reservations pre-empted (Unit Attention)",
    "06,2A,04,Unit Attention - Reservations released (Unit Attention)",
    "06,2A,05,Unit Attention - Registrations pre-empted (Unit Attention)",
    "06,2F,00,Unit Attention - commands cleared by another initiator (Unit Attention)",
    "06,3F,00,Unit Attention - target operating conditions have changed (Unit Attention)",
    "06,3F,01,Unit Attention - microcode changed (Unit Attention)",
    "06,3F,02,Unit Attention - changed operating definition (Unit Attention)",
    "06,3F,03,Unit Attention - inquiry parameters changed (Unit Attention)",
    "06,3F,05,Unit Attention - device identifier changed (Unit Attention)",
    "06,5D,00,Unit Attention - PFA threshold reached (Unit Attention)",
    "07,27,00,Write Protect - command not allowed (Write Protect)",
    "0B,00,00,Aborted Command - no additional sense code",
    "0B,00,1E,Aborted Command - Sanitize command aborted",
    "0B,1B,00,Aborted Command - sync data transfer error (extra ACK) (Aborted Command)",
    "0B,25,00,Aborted Command - unsupported LUN (Aborted Command)",
    "0B,3F,0F,Aborted Command - echo buffer overwritten (Aborted Command)",
    "0B,43,00,Aborted Command - message reject error (Aborted Command)",
    "0B,44,00,Aborted Command - internal target failure (Aborted Command)",
    "0B,45,00,Aborted Command - Selection/Reselection failure (Aborted Command)",
    "0B,47,00,Aborted Command - SCSI parity error (Aborted Command)",
    "0B,48,00,Aborted Command - initiator-detected error message received (Aborted Command)",
    "0B,49,00,Aborted Command - inappropriate/illegal message (Aborted Command)",
    "0B,4B,00,Aborted Command - data phase error (Aborted Command)",
    "0B,4E,00,Aborted Command - overlapped commands attempted (Aborted Command)",
    "0B,4F,00,Aborted Command - due to loop initialisation (Aborted Command)",
    "0E,1D,00,Miscompare - during verify byte check operation (Aborted Command)",
    "0x,05,00,Illegal request (Other)",
    "0x,06,00,Unit attention (Other)",
    "0x,07,00,Data protect (Other)",
    "0x,08,00,LUN communication failure (Other)",
    "0x,08,01,LUN communication timeout (Other)",
    "0x,08,02,LUN communication parity error (Other)",
    "0x,08,03,LUN communication CRC error (Other)",
    "0x,09,00,vendor specific sense key (Other)",
    "0x,09,01,servo fault (Other)",
    "0x,09,04,head select fault",
    "0x,0A,00,error log overflow",
    "0x,0B,00,aborted command",
    "0x,0C,00,write error",
    "0x,0C,02,write error - auto-realloc failed",
    "0x,0E,00,data miscompare",
    "0x,12,00,address mark not found for ID field",
    "0x,14,00,logical block not found",
    "0x,15,00,random positioning error",
    "0x,15,01,mechanical positioning error",
    "0x,15,02,positioning error detected by read of medium",
    "0x,27,00,write protected",
    "0x,29,00,POR or bus reset occurred",
    "0x,31,01,format failed",
    "0x,32,01,defect list update error",
    "0x,32,02,no spares available",
    "0x,35,01,unspecified enclosure services failure",
    "0x,37,00,parameter rounded",
    "0x,3D,00,invalid bits in identify message",
    "0x,3E,00,LUN not self-configured yet",
    "0x,40,01,DRAM parity error",
    "0x,40,02,DRAM parity error",
    "0x,42,00,power-on or self-test failure",
    "0x,4C,00,LUN failed self-configuration",
    "0x,5C,00,RPL status change",
    "0x,5C,01,spindles synchronised",
    "0x,5C,02,spindles not synchronised",
    "0x,65,00,voltage fault",
    "0x,80,x,Vendor specific",
    "0x,x,80,Vendor specific",
    "00,00,00,No Additional Sense Information (SEAGATE Documentation)",
    "01,03,00,Peripheral Device Write Fault (SEAGATE Documentation)",
    "01,09,00,Track Following Error (SEAGATE Documentation)",
    "01,09,00,Track Following Error (SEAGATE Documentation)",
    "01,09,01,Servo Fault (SEAGATE Documentation)",
    "01,09,0D,Write to at least one copy of a redundant file failed (SEAGATE Documentation)",
    "01,09,0E,Redundant files have < 50% good copies (SEAGATE Documentation)",
    "01,09,F8,Calibration is needed but the QST is set without the Recal Only bit (SEAGATE Documentation)",
    "01,09,FF,Servo Cal completed as part of self-test (SEAGATE Documentation)",
    "01,0B,01,Warning-Specified Temperature Exceeded (SEAGATE Documentation)",
    "01,0B,02,Warning,Enclosure Degraded (SEAGATE Documentation)",
    "01,0C,01,Write Error Recovered With Auto-Reallocation (SEAGATE Documentation)",
    "01,11,00,Unrecovered Read Error (SEAGATE Documentation)",
    "01,15,01,Mechanical Positioning Error (SEAGATE Documentation)",
    "01,16,00,Data Synchronization Mark Error (SEAGATE Documentation)",
    "01,17,01,Recovered Data Using Retries (SEAGATE Documentation)",
    "01,17,02,Recovered Data Using Positive Offset (SEAGATE Documentation)",
    "01,17,03,Recovered Data Using Negative Offset (SEAGATE Documentation)",
    "01,18,00,Recovered Data With ECC (SEAGATE Documentation)",
    "01,18,01,Recovered Data With ECC And Retries Applied (SEAGATE Documentation)",
    "01,18,02,Recovered Data With ECC And/Or Retries,Data Auto-Reallocated (SEAGATE Documentation)",
    "01,18,07,Recovered Data With ECC-Data Rewritten (SEAGATE Documentation)",
    "01,19,00,Defect List Error (SEAGATE Documentation)",
    "01,1C,00,Defect List Not Found (SEAGATE Documentation)",
    "01,1F,00,Number of Defects Overflows the Allocated Space That The Read Defect Command Can Handle (SEAGATE Documentation)",
    "01,37,00,Parameter Rounded (SEAGATE Documentation)",
    "01,3F,80,Buffer contents have changed (SEAGATE Documentation)",
    "01,40,01,DRAM Parity Error (SEAGATE Documentation)",
    "01,40,02,Spinup Error recovered with retries (SEAGATE Documentation)",
    "01,44,00,Internal Target Failure (SEAGATE Documentation)",
    "01,5D,00,Failure Prediction Threshold Exceeded (SEAGATE Documentation)",
    "01,5D,FF,False Failure Prediction Threshold Exceeded (SEAGATE Documentation)",
    "02,04,00,Logical Unit Not Ready,Cause Not Reportable (SEAGATE Documentation)",
    "02,04,01,Logical Unit Not Ready,Becoming Ready (SEAGATE Documentation)",
    "02,04,02,Logical Unit Not Ready,SMART UNIT Required (SEAGATE Documentation)",
    "02,04,03,Logical Unit Not Ready,Manual Intervention Required (SEAGATE Documentation)",
    "02,04,04,Logical Unit Not Ready,Format in Progress (SEAGATE Documentation)",
    "02,04,09,Logical Unit Not Ready,Self Test in Progress (SEAGATE Documentation)",
    "02,04,0A,Logical Unit Not Ready,NVC recovery in progress after and exception event (SEAGATE Documentation)",
    "02,04,11,Logical Unit Not Ready,Notify (Enable Spinup) required (SEAGATE Documentation)",
    "02,04,F0,Logical unit not ready,super certify in progress (SEAGATE Documentation)",
    "02,35,02,Enclosure Services Unavailable (SEAGATE Documentation)",
    "03,03,00,Peripheral Device Write Fault (SEAGATE Documentation)",
    "03,09,00,Track Following Error (SEAGATE Documentation)",
    "03,09,00,Track Following Error (SEAGATE Documentation)",
    "03,09,04,Head Select Fault (SEAGATE Documentation)",
    "03,0A,01,Failed to write super certify log file (SEAGATE Documentation)",
    "03,0A,02,Failed to read super certify log file (SEAGATE Documentation)",
    "03,0C,00,Write Error (SEAGATE Documentation)",
    "03,0C,02,Write Error-Auto Reallocation Failed (SEAGATE Documentation)",
    "03,0C,03,Write Error-Recommend Reassignment (SEAGATE Documentation)",
    "03,0C,FF,Write Error-Too many error recovery revs (SEAGATE Documentation)",
    "03,11,00,Unrecovered Read Error (SEAGATE Documentation)",
    "03,11,04,Unrecovered Read Error-Auto Reallocation Failed (SEAGATE Documentation)",
    "03,11,FF,Unrecovered Read Error-Too many error recovery revs (SEAGATE Documentation)",
    "03,14,01,Record Not Found (SEAGATE Documentation)",
    "03,15,01,Mechanical Positioning Error (SEAGATE Documentation)",
    "03,16,00,Data Synchronization Mark Error (SEAGATE Documentation)",
    "03,31,00,Medium Format Corrupted (SEAGATE Documentation)",
    "03,31,01,Corruption in R/W format request (SEAGATE Documentation)",
    "03,31,91,Corrupt World Wide Name (WWN) in drive information file (SEAGATE Documentation)",
    "03,32,01,Defect List Update Error (SEAGATE Documentation)",
    "03,32,03,Defect list longer than allocated memory (SEAGATE Documentation)",
    "03,33,00,Flash not ready for access (SEAGATE Documentation)",
    "03,44,00,Internal Target Failure (SEAGATE Documentation)",
    "04,01,00,No Index/Logical Block Signal (SEAGATE Documentation)",
    "04,02,00,No SEEK Complete (SEAGATE Documentation)",
    "04,03,00,Peripheral Device Write Fault (SEAGATE Documentation)",
    "04,09,00,Track Following Error (SEAGATE Documentation)",
    "04,09,00,Track Following Error (SEAGATE Documentation)",
    "04,09,01,Servo Fault (SEAGATE Documentation)",
    "04,09,04,Head Select Fault (SEAGATE Documentation)",
    "04,15,01,Mechanical Positioning Error (SEAGATE Documentation)",
    "04,16,00,Data Synchronization Mark Error (SEAGATE Documentation)",
    "04,19,00,Defect List Error (SEAGATE Documentation)",
    "04,1C,00,Defect List Not Found (SEAGATE Documentation)",
    "04,32,01,Defect List Update Error (SEAGATE Documentation)",
    "04,40,01,DRAM Parity Error (SEAGATE Documentation)",
    "04,44,00,Internal Target Failure (SEAGATE Documentation)",
    "05,05,00,ILLEGAL REQUEST (SEAGATE Documentation)",
    "05,1A,00,Parameter List Length Error (SEAGATE Documentation)",
    "05,20,00,Invalid Command Operation Code (SEAGATE Documentation)",
    "05,20,F3,Invalid linked command operation code (SEAGATE Documentation)",
    "05,24,00,Invalid Field In CDB (SEAGATE Documentation)",
    "05,24,01,Illegal Queue Type for CDB (Low priority commands must be SIMPLE queue) (SEAGATE Documentation)",
    "05,24,F0,Invalid LBA in linked command (SEAGATE Documentation)",
    "05,24,F2,Invalid linked command operation code (SEAGATE Documentation)",
    "05,24,F3,Illegal G->P operation request (SEAGATE Documentation)",
    "05,25,00,Logical Unit Not Supported (SEAGATE Documentation)",
    "05,26,00,Invalid Field In Parameter List (SEAGATE Documentation)",
    "05,26,01,Parameter Not Supported (SEAGATE Documentation)",
    "05,26,02,Parameter Value Invalid (SEAGATE Documentation)",
    "05,26,03,Invalid Field Parameter-Threshold Parameter (SEAGATE Documentation)",
    "05,26,04,Invalid Release of Active Persistent Reserve (SEAGATE Documentation)",
    "05,26,05,Fail to read valid log dump data (SEAGATE Documentation)",
    "05,2C,00,Command Sequence Error (SEAGATE Documentation)",
    "05,32,01,Defect List Update Error (SEAGATE Documentation)",
    "05,35,01,Unsupported Enclosure Function (SEAGATE Documentation)",
    "05,55,04,PRKT table is full (SEAGATE Documentation)",
    "06,06,00,UNIT ATTENTION (SEAGATE Documentation)",
    "06,0B,01,Warning-Specified Temperature Exceeded (SEAGATE Documentation)",
    "06,29,00,Power On,Reset,Or Bus Device Reset Occurred (SEAGATE Documentation)",
    "06,29,01,Power-On Reset Occurred (SEAGATE Documentation)",
    "06,29,02,SCSI Bus Reset Occurred (SEAGATE Documentation)",
    "06,29,03,Bus Device Reset Function Occurred (SEAGATE Documentation)",
    "06,29,04,Internal Reset Occurred (SEAGATE Documentation)",
    "06,29,05,Transceiver Mode Changed To Single-Ended (SEAGATE Documentation)",
    "06,29,06,Transceiver Mode Changed To LVD (SEAGATE Documentation)",
    "06,29,07,Write Log Dump data to disk successful OR IT Nexus Loss (SEAGATE Documentation)",
    "06,29,08,Write Log Dump data to disk fail (SEAGATE Documentation)",
    "06,29,09,Write Log Dump Entry information fail (SEAGATE Documentation)",
    "06,29,0A,Reserved disc space is full (SEAGATE Documentation)",
    "06,29,0B,SDBP test service contained an error,examine status packet(s) for details (SEAGATE Documentation)",
    "06,29,0C,SDBP incoming buffer overflow (incoming packet too big) (SEAGATE Documentation)",
    "06,29,CD,Flashing LED occurred. (Cold reset) (SEAGATE Documentation)",
    "06,29,CE,Flashing LED occurred. (Warm reset) (SEAGATE Documentation)",
    "06,2A,01,Mode Parameters Changed (SEAGATE Documentation)",
    "06,2A,02,Log Parameters Changed (SEAGATE Documentation)",
    "06,2A,03,Reservations preempted (SEAGATE Documentation)",
    "06,2A,04,Reservations Released (SEAGATE Documentation)",
    "06,2A,05,Registrations Preempted (SEAGATE Documentation)",
    "06,2F,00,Tagged Commands Cleared By Another Initiator (SEAGATE Documentation)",
    "06,3F,00,Target Operating Conditions Have Changed (SEAGATE Documentation)",
    "06,3F,01,Device internal reset occurred (SEAGATE Documentation)",
    "06,3F,02,Changed Operating Definition (SEAGATE Documentation)",
    "06,3F,05,Device Identifier Changed (SEAGATE Documentation)",
    "06,3F,91,World Wide Name (WWN) Mismatch (SEAGATE Documentation)",
    "06,5C,00,RPL Status Change (SEAGATE Documentation)",
    "06,5D,00,Failure Prediction Threshold Exceeded (SEAGATE Documentation)",
    "06,5D,FF,False Failure Prediction Threshold Exceeded (SEAGATE Documentation)",
    "06,B4,00,Unreported Deferred Errors have been logged on log page 34h (SEAGATE Documentation)",
    "07,07,00,Data Protect (SEAGATE Documentation)",
    "07,27,00,Write Protected (SEAGATE Documentation)",
    "09,08,00,Logical Unit Communication Failure (SEAGATE Documentation)",
    "09,80,00,General Firmware Error Qualifier (SEAGATE Documentation)",
    "09,80,86,IOEDC Error on Read (SEAGATE Documentation)",
    "09,80,87,IOEDC Error on Write (SEAGATE Documentation)",
    "09,80,88,Host Parity Check Failed (SEAGATE Documentation)",
    "09,80,89,IOEDC Error on Read Detected by Formatter (SEAGATE Documentation)",
    "09,80,8A,Host FIFO Parity Error detected by Common Buffer (SEAGATE Documentation)",
    "09,80,8B,Host FIFO Parity Error detected by frame buffer logic (SEAGATE Documentation)",
    "09,80,8C,Host Data Frame Buffer Parity Error (SEAGATE Documentation)",
    "0B,08,00,Logical Unit Communication Failure (SEAGATE Documentation)",
    "0B,08,01,Logical Unit Communication Time-Out (SEAGATE Documentation)",
    "0B,0B,00,Aborted Command (SEAGATE Documentation)",
    "0B,3F,0F,Echo buffer overwritten (SEAGATE Documentation)",
    "0B,43,00,Message Reject Error (SEAGATE Documentation)",
    "0B,45,00,Select/Reselection Failure (SEAGATE Documentation)",
    "0B,47,00,SCSI Parity Error (SEAGATE Documentation)",
    "0B,47,03,Information Unit CRC Error (SEAGATE Documentation)",
    "0B,47,80,Fibre Channel Sequence Error (SEAGATE Documentation)",
    "0B,48,00,Initiator Detected Error Message Received (SEAGATE Documentation)",
    "0B,49,00,Invalid Message Received (SEAGATE Documentation)",
    "0B,4B,00,Data Phase Error (SEAGATE Documentation)",
    "0B,4B,01,Invalid transfer tag (SEAGATE Documentation)",
    "0B,4B,02,Too many write data (SEAGATE Documentation)",
    "0B,4B,03,ACK NAK Timeout (SEAGATE Documentation)",
    "0B,4B,04,NAK received (SEAGATE Documentation)",
    "0B,4B,05,Data Offset error (SEAGATE Documentation)",
    "0B,4B,06,Initiator response timeout (SEAGATE Documentation)",
    "0B,4E,00,Overlapped Commands Attempted (SEAGATE Documentation)",
    "0B,81,00,LA Check Error (SEAGATE Documentation)",
    "0D,0D,00,Volume Overflow Constants (SEAGATE Documentation)",
    "0D,21,00,Logical Block Address Out Of Range (SEAGATE Documentation)",
    "0E,0E,00,Data Miscompare (SEAGATE Documentation)",
    "0E,1D,00,Miscompare During Verify Operation (SEAGATE Documentation)",
    "ANY,03,86,Write Fault Data Corruption (SEAGATE Documentation)",
    "ANY,08,02,Logical Unit Communication Parity Error (SEAGATE Documentation)",
    "ANY,0A,00,Error Log Overflow (SEAGATE Documentation)",
    "ANY,10,00,ID CRC Or ECC Error (SEAGATE Documentation)",
    "ANY,11,01,Read Retries Exhausted (SEAGATE Documentation)",
    "ANY,11,02,Error Too Long To Correct (SEAGATE Documentation)",
    "ANY,12,00,Address Mark Not Found For ID Field (SEAGATE Documentation)",
    "ANY,12,01,Recovered Data Without ECC Using Previous Logical Block ID (SEAGATE Documentation)",
    "ANY,12,02,Recovered Data With ECC Using Previous Logical Block ID (SEAGATE Documentation)",
    "ANY,14,00,Logical Block Not Found (SEAGATE Documentation)",
    "ANY,15,00,Random Positioning Error (SEAGATE Documentation)",
    "ANY,15,02,Positioning Error Detected By Read Of Medium (SEAGATE Documentation)",
    "ANY,17,00,Recovered Data With No Error Correction Applied (SEAGATE Documentation)",
    "ANY,17,05,Recovered Data Using Previous Logical Block ID (SEAGATE Documentation)",
    "ANY,17,06,Recovered Data Without ECC-Data Auto Reallocated (SEAGATE Documentation)",
    "ANY,18,05,Recovered Data-Recommand Reassignment (SEAGATE Documentation)",
    "ANY,18,06,Recovered Data Using ECC and Offsets (SEAGATE Documentation)",
    "ANY,19,01,Defect List Not Available (SEAGATE Documentation)",
    "ANY,19,02,Defect List Error In Primary List (SEAGATE Documentation)",
    "ANY,19,03,Defect List Error in Grown List (SEAGATE Documentation)",
    "ANY,19,0E,Fewer than 50% Defect List Copies (SEAGATE Documentation)",
    "ANY,1B,00,Synchronous Data Transfer Error (SEAGATE Documentation)",
    "ANY,1C,01,Primary Defect List Not Found (SEAGATE Documentation)",
    "ANY,1C,02,Grown Defect List Not Found (SEAGATE Documentation)",
    "ANY,1C,83,Seagate Unique Diagnostic Code (SEAGATE Documentation)",
    "ANY,26,97,Invalid Field Parameter-TMS Firmware Tag (SEAGATE Documentation)",
    "ANY,26,98,Invalid Field Parameter-Check Sum (SEAGATE Documentation)",
    "ANY,26,99,Invalid Field Parameter-Firmware Tag (SEAGATE Documentation)",
    "ANY,29,00,Flashing LED occurred (SEAGATE Documentation)",
    "ANY,32,00,No Defect Spare Location Available (SEAGATE Documentation)",
    "ANY,32,02,No Spares Available-Too Many Defects On One Track (SEAGATE Documentation)",
    "ANY,35,00,Unspecified Enclosure Services Failure (SEAGATE Documentation)",
    "ANY,35,03,Enclosure Transfer Failure (SEAGATE Documentation)",
    "ANY,35,04,Enclosure Transfer Refused (SEAGATE Documentation)",
    "ANY,3D,00,Invalid Bits In Identify Message (SEAGATE Documentation)",
    "ANY,3E,00,Logical Unit Has Not Self Configured Yet (SEAGATE Documentation)",
    "ANY,3E,03,Logical Unit Failed Self Test (SEAGATE Documentation)",
    "ANY,3F,90,Invalid APM Parameters (SEAGATE Documentation)",
    "ANY,42,00,Power-On Or Self-Test Failure (SEAGATE Documentation)",
    "ANY,42,0A,Port A failed loopback test (SEAGATE Documentation)",
    "ANY,42,0B,Port B failed loopback test (SEAGATE Documentation)",
    "ANY,44,F2,Data Integrity Check Failed on verify (SEAGATE Documentation)",
    "ANY,44,F6,Data Integrity Check Failed during write (SEAGATE Documentation)",
    "ANY,44,FF,XOR CDB check error (SEAGATE Documentation)",
    "ANY,4C,00,Logical Unit Failed Self-Configuration (SEAGATE Documentation)",
    "ANY,55,01,XOR Cache is Not Available (SEAGATE Documentation)",
    "ANY,5B,00,Log Exception (SEAGATE Documentation)",
    "ANY,5B,01,Threshold Condition Met (SEAGATE Documentation)",
    "ANY,5B,02,Log Counter At Maximum (SEAGATE Documentation)",
    "ANY,5B,03,Log List Codes Exhausted (SEAGATE Documentation)",
    "ANY,5C,01,Spindles Synchronized (SEAGATE Documentation)",
    "ANY,5C,02,Spindles Not Synchronized (SEAGATE Documentation)",
    "ANY,65,00,Voltage Fault (SEAGATE Documentation)",
    "ANY,81,00,LA Check Error,LCM bit = 0 (SEAGATE Documentation)",
    "ANY,81,00,Reassign Power-Fail Recovery Failed (SEAGATE Documentation)",
    "00,00,00,No Additional Sense Information No valid additional sense information is present. (Hitachi Documentation)",
    "04,01,00,No Index/Sector Signal No Index signal could be detected.  No Sector signal could be detected. (Hitachi Documentation)",
    "04,02,00,No Seek Complete The drive seek was not completed successfully. (Hitachi Documentation)",
    "01,02,00,No Seek Complete The drive seek was not completed successfully. (Hitachi Documentation)",
    "04,02,80,Unexpected Carriage Unload Unexpected carriage un lock error was occurred. (Hitachi Documentation)",
    "01,02,80,Unexpected Carriage Unload Unexpected carriage un lock error was occurred. (Hitachi Documentation)",
    "04,02,C9,Seek Measure Failed (Lower Limit) A seek measure value less than lower limit (Hitachi Documentation)",
    "04,02,CA,Seek Measure failed (Upper Limit) A seek measure value over than upper limit value (Hitachi Documentation)",
    "01,03,00,Peripheral Device Write Fault A write fault was detected on the drive. (Hitachi Documentation)",
    "04,03,00,Peripheral Device Write Fault A write fault was detected on the drive. (Hitachi Documentation)",
    "01,03,80,Write Fault on Write Inhibit Condition A write fault by issuing WRITE command on write inhibit condition. (Hitachi Documentation)",
    "04,03,80,Write Fault on Write Inhibit Condition A write fault by issuing WRITE command on write inhibit condition. (Hitachi Documentation)",
    "04,03,89,Write Cylinder Number Error A cylinder number injustice was detected after write end. (Hitachi Documentation)",
    "01,03,DA,Servo Window Error A servo window error occurred. (Hitachi Documentation)",
    "04,03,DA,Servo Window Error A servo window error occurred. (Hitachi Documentation)",
    "02,04,00,Logical Unit Not Ready , Cause Not Reportable No drive Ready signal could be detected.  The specified drive could not be accessed. (Hitachi Documentation)",
    "02,04,01,Logical Unit is in Process of Becoming Ready.  Drive is not ready but to be ready soon. (Hitachi Documentation)",
    "02,04,02,Logical Unit Not Ready , Initializing Command Required Drive is not ready and waiting for START UNIT Command. (Hitachi Documentation)",
    "02,04,04,Logical Unit Not Ready , Format in Progress Drive is not ready because Format Unit is in progress.  (Hitachi Documentation)",
    "02,04,09,Logical Unit Not Ready , Self-test in Progress Drive is not ready because Self- Test is in progress. (Hitachi Documentation)",
    "02,04,84,Logical Unit Not Ready , ETF in Progress Drive is not ready because ETF is in progress. (Hitachi Documentation)",
    "04,08,00,Logical Unit Communication Failure A logical unit interface error occurred. (Hitachi Documentation)",
    "04,08,01,Logical Unit Communication Time Out A logical unit interface time out error occurred. (Hitachi Documentation)",
    "04,08,02,Logical Unit Communication Parity Error A logical unit interface parity error occurred. (Hitachi Documentation)",
    "04,08,81,Drive Fault with ATN OFF A drive fault was detected on ESDI ATN=0. (Hitachi Documentation)",
    "01,08,81,Drive Fault with ATN OFF A drive fault was detected on ESDI ATN=0. (Hitachi Documentation)",
    "04,08,83,Drive Error in non Error Factor Drive error was detected, but there were none error factor. (Hitachi Documentation)",
    "04,09,00,Track Following Error Track Positioning was failed. (Hitachi Documentation)",
    "01,09,00,Track Following Error Track Positioning was failed. (Hitachi Documentation)",
    "01,09,04,Head Select Fault Head Select was failed. (Hitachi Documentation)",
    "04,09,04,Head Select Fault Head Select was failed. (Hitachi Documentation)",
    "01,09,80,Track Positioning Error Track Positioning was failed between confirmation of ATN off and issue of MESDI command related to seek. (Hitachi Documentation)",
    "04,09,80,Track Positioning Error Track Positioning was failed between confirmation of ATN off and issue of MESDI command related to seek. (Hitachi Documentation)",
    "00,0B,01,Specified Temperature Exceeded Temperature value gotten from sensor was over its threshold value. (Hitachi Documentation)",
    "01,0B,01,Specified Temperature Exceeded Temperature value gotten from sensor was over its threshold value. (Hitachi Documentation)",
    "01,0C,01,Write Error Recovered with Auto Reallocation A write error has been recovered by sector relocation. Auto-Reallocation process was performed. (Hitachi Documentation)",
    "03,0C,02,Write Error-Auto Reallocation Failed A write error has not been recovered.  An Auto-Reallocation for a write error was not successful. (Hitachi Documentation)",
    "01,0C,81,Write Error Recovered with Auto Reallocation, Relocation Threshold- Over A write error has been recovered by sector relocation. Auto-Reallocation process was performed but a relocation count was over its threshold value. (Hitachi Documentation)",
    "03,0C,82,Write Error Auto Reallocation Not Execute with Time Out An Auto Reallocation for a write error was not execute with time out. (Hitachi Documentation)",
    "02,73,3,0C FF Write Command Terminate with Recovery Time Out Write processing time exceeded Recovery Time Limit, write processing was terminated.. (Hitachi Documentation)",
    "03,10,00,ID CRC or ECC Error A CRC error occurred in an ID field. (Hitachi Documentation)",
    "03,11,00,Unrecovered Read Error A read error occurred in a data field. (Retries are not applied.) (Hitachi Documentation)",
    "03,11,01,Read Retries Exhausted A read error in a data field could not be recovered by retries. (Error correction is not applied.) (Hitachi Documentation)",
    "03,11,02,Error Too Long to Correct A read error in data field could not be corrected by ECC. (Hitachi Documentation)",
    "03,11,04,Unrecovered Read Error-Auto Reallocation Failed An auto reallocation for a read error was not successful. (Hitachi Documentation)",
    "03,11,0A,Miscorrected Error A read error in LBA/CRC field could not be corrected by ECC. (Hitachi Documentation)",
    "03,11,0B,Unrecoverd Read Error-Recommend Reassignment A read error in data field could not be corrected by ECC.  A reassignment is recommended. (Hitachi Documentation)",
    "03,11,82,Error Too Long to Multi Symbol Soft Correction The data field read error could not be corrected using soft correction. (Hitachi Documentation)",
    "03,11,84,Read Error Auto Reallocation Not Execute with Time Out An Auto Reallocation for a Read error was not execute with time out. (Hitachi Documentation)",
    "03,11,D7,Uncorrectable Check Code ECC Error A read error in check code field could not be corrected by ECC. (Hitachi Documentation)",
    "03,11,FF,Read or Verify Command Terminate with Recovery Time Out Read or Verify processing time exceeded Recovery Time Limit, read or verify processing was terminated.. (Hitachi Documentation)",
    "03,12,00,Address Mark Not Found for ID Field No address mark could be found in an ID field. (Hitachi Documentation)",
    "01,13,00,Address Mark Not Found for Data Field No address mark could be found in a data field. (Hitachi Documentation)",
    "03,13,00,Address Mark Not Found for Data Field No address mark could be found in a data field. (Hitachi Documentation)",
    "01,13 80 Split Data AM Not Found No address mark could be found in a split data. (Hitachi Documentation)",
    "03,13 80 Split Data AM Not Found No address mark could be found in a split data. (Hitachi Documentation)",
    "03,13,80,Split Data AM Not Found No address mark could be found in a split data. (Hitachi Documentation)",
    "01,14,00,Recorded Entity Not Found No recorded entity could be found.  OEM MANUAL (Hitachi Documentation)",
    "03,14,00,Recorded Entity Not Found No recorded entity could be found.  OEM MANUAL (Hitachi Documentation)",
    "01,17,06,Recovered Data without ECC - Data Auto-Reallocated An error has been recovered by retries (without ECC) and a sector has been reallocated. Auto-Reallocation process was performed. (Hitachi Documentation)",
    "01,17,07,Recovered Data without ECC - Recommend Reassignment The error has been recovered without ECC correction. A reassignment is recommended. (Hitachi Documentation)",
    "01,17,09,Recovered Data without ECC- Data Rewritten The error has been recovered without ECC correction and data rewritten. (Hitachi Documentation)",
    "01,17,86,Recovered Data without ECC - Data Auto-Reallocated, Relocation Threshold-Over Data error has been recovered by ECC correction and a sector has been reallocated.  Auto-Reallocation process was performed but a relocation count was over its threshold value. (Hitachi Documentation)",
    "01,18,00,Recovered Data with Error Correction Applied Data error has been recovered by ECC correction. (without retries) (Hitachi Documentation)",
    "01,18,01,Recovered Data with Error Correction & Retries Applied Data error has been recovered by ECC correction and retries. (Hitachi Documentation)",
    "01,18,02,Recovered Data - Data Auto- Reallocated Data error has been recovered by ECC correction (retries may or may not be done) and a sector has been reallocated.  Auto- Reallocation process was performed. (Hitachi Documentation)",
    "01,18,05,Recovered Data- Recommend Reassignment Data error has been recovered by ECC correction. A reassignment is recommended. (Hitachi Documentation)",
    "01,18,07,Recovered Data with ECC-Data Rewritten Data error has been recovered by ECC correction and data rewritten . (Hitachi Documentation)",
    "01,18,82,Recovered Data - Data Auto- Reallocated, Relocation Threshold-Over Data error has been recovered by ECC correction and a sector has been reallocated.  Auto-Reallocation process was performed but a relocation count was over its threshold value. (Hitachi Documentation)",
    "01,19,00,Defect List error Error exists in defect list. (Hitachi Documentation)",
    "03,19,00,Defect List error Error exists in defect list. (Hitachi Documentation)",
    "01,19,01,Defect List Error Not Available Defect list could not be used. (Hitachi Documentation)",
    "03,19,02,Defect List Error in Primary List An error occurred during an access to the Primary (P) list (Hitachi Documentation)",
    "01,19,03,Defect List Error in Grown List An error occurred during an access to the Grown (G) list. (Hitachi Documentation)",
    "03,19,03,Defect List Error in Grown List An error occurred during an access to the Grown (G) list. (Hitachi Documentation)",
    "05,1A,00,Parameter List Length Error A parameter list length is incorrect. (Hitachi Documentation)",
    "0B,1B,00,Synchronous Data Transfer Error An error occurred in synchronous data transfer. (Hitachi Documentation)",
    "01,1C,00,Defect List Not Found Defect list could not be detected. (Hitachi Documentation)",
    "03,1C,00,Defect List Not Found Defect list could not be detected. (Hitachi Documentation)",
    "03,1C,01,Primary Defect List Not Found An access to the Primary (P) list has failed. (Hitachi Documentation)",
    "01,1C,01,Primary Defect List Not Found An access to the Primary (P) list has failed. (Hitachi Documentation)",
    "03,1C,02,Grown Defect List Not Found An access to the Grown (G) list has failed. (Hitachi Documentation)",
    "01,1C,02,Grown Defect List Not Found An access to the Grown (G) list has failed. (Hitachi Documentation)",
    "0E,1D,00,Miscompare During Verify Operation A data compare error occurred during a verification process. (Hitachi Documentation)",
    "05,20,00,Invalid Command Operation Code An invalid operation code was specified. (Hitachi Documentation)",
    "05,21,00,Logical Block Address out of Range An attempt was made to access beyond the Logical Block Address reported by a READ CAPACITY command (with the PMI bit set to 0). (Hitachi Documentation)",
    "05,24,00,Illegal Field in CDB An illegal data was specified in the CDB.  E.g.; Reserved bit/Value of non-zero, or Unsupported bit/Value of non-zero. (Hitachi Documentation)",
    "05,24,80,Download in Progress A command cannot be executed. (The download in progress.) (Hitachi Documentation)",
    "05,24,81,Odd Byte Data Out Request in Wide XFR Odd byte data out request occurred in wide transfer. (Hitachi Documentation)",
    "05,25,00,Invalid LUN An unimplemented LUN was specified in the CDB or Identify message. (Hitachi Documentation)",
    "05,26,00,Invalid Field in Parameter List An invalid field was specified in a parameter list. (Hitachi Documentation)",
    "05,26,01,Parameter Not Supported An unsupported parameter is received. (Hitachi Documentation)",
    "05,26,02,Parameter Value Invalid A parameter value is invalid. (Hitachi Documentation)",
    "05,26,03,Threshold Parameters Not Supported An unsupported threshold parameters is received. (Hitachi Documentation)",
    "05,26,04,Invalid Release of Persistent Reservation An invalid release of persistent reservation. (Hitachi Documentation)",
    "05,26,80,Microprogram Download Error Different file from Inquiry type was downloaded in microprogram downloading. (Hitachi Documentation)",
    "07,27,00,Write Protected The specified drive was write-protected. (Hitachi Documentation)",
    "06,29,00,Power On or Reset or Bus Device Reset Occurred A power-on reset occurred.  A Bus Device Reset message was issued.  An SCSI bus reset occurred.  (Not Occurred) (Hitachi Documentation)",
    "06,29,01,Power On Reset Occurred A power-on reset occurred. (Hitachi Documentation)",
    "06,29,02,SCSI Bus Reset Occurred An SCSI bus reset occurred. (Hitachi Documentation)",
    "06,29,03,Bus Device Reset Occurred A Bus Device Reset message was issued. (Hitachi Documentation)",
    "06,29,04,Device Internal Reset A device internal reset occurred. (Hitachi Documentation)",
    "06,29,05,Mode Changed from LVD to SE Mode changed from LVD to SE. (Hitachi Documentation)",
    "06,29,06,Mode Changed from SE to LVD Mode changed from SE to LVD. (Hitachi Documentation)",
    "06,2A,00,Parameters Changed The Mode/Log parameters were altered. E.g., Mode Select command altered parameters.  E.g., Mode/Log parameters were reset by a Not Ready to Ready transition of the drive. (Hitachi Documentation)",
    "02,2A,01,Mode Parameters Changed The mode parameter has been changed. (Hitachi Documentation)",
    "06,2A,01,Mode Parameters Changed The mode parameter has been changed. (Hitachi Documentation)",
    "06,2A,02,Log Parameters Changed The Log parameters have been changed. (Hitachi Documentation)",
    "06,2A,03,Reservations Preempted The reservation key has been cleared. (Hitachi Documentation)",
    "06,2A,04,Reservations Released The reservation has been cleared. (Hitachi Documentation)",
    "06,2A,05,Registrations Preempted The persistent reservation has been preempted. (Hitachi Documentation)",
    "06,2F,00,Commands Cleared by Another Initiator The executing or queuing commands have (Hitachi Documentation)",
    "02,31,00,Medium Format Corrupted The medium has not been formatted properl y.  It is necessary to reformat the medium with a FORMAT UNIT command. (Hitachi Documentation)",
    "03,31,00,Medium Format Corrupted The medium has not been formatted properl y.  It is necessary to reformat the medium with a FORMAT UNIT command. (Hitachi Documentation)",
    "02,31,01,Format Command Failed A Format command completed in the abnormal condition.  It is necessary to reformat the medium with a FORMAT UNIT command. (Hitachi Documentation)",
    "03,31,01,Format Command Failed A Format command completed in the abnormal condition.  It is necessary to reformat the medium with a FORMAT UNIT command. (Hitachi Documentation)",
    "01,31,F0,ETF Cylinder Read Error The controller detected read error during reading ETF in initial format and judged (Hitachi Documentation)",
    "01,31,F1,ETF Cylinder ESDI Defect List Check Error The controller detected ESDI defect list check error during initial format and judged there was no P-list. (Hitachi Documentation)",
    "04,32,00,No Defect Spare Location Available Due to short alternate spares, reassigning blocks could not be processed. (Hitachi Documentation)",
    "03,32,01,Defect List Update Failure An updating the Grown (G) list failed. (Hitachi Documentation)",
    "01,37,00,Rounded Parameter A parameter value received was not useable as it was, so it was rounded by the controller. (Hitachi Documentation)",
    "05,3D,00,Invalid Bits in Identify Message The invalid bits were detected in Identify (Hitachi Documentation)",
    "02,3E,00,Logical Unit Has Not Self-configured Y et The Self-configuration of Logical Unit has not finished yet. (Hitachi Documentation)",
    "04,3E,03,Logical Unit Failed Self- Test Failed to Self- Test. (Hitachi Documentation)",
    "04,3E,04,Logical Unit Unable to Update Self- Test Log Unable to update the Self- Test log. (Hitachi Documentation)",
    "06,3F,00,Target Operating Conditions Have Changed The operating conditions of target have changed. (Hitachi Documentation)",
    "06,3F,01,Microcode Has Been Changed A micro code has been changed. (Hitachi Documentation)",
    "06,3F,02,Changed Operating Definition An operating definition has been changed. (Hitachi Documentation)",
    "06,3F,03,Inquiry Data Has Changed A Inquiry data has changed. (Hitachi Documentation)",
    "04,40,00,RAM Failure Failure of RAM memor y. (Hitachi Documentation)",
    "04,42,00,Power On or Self- test Failure A power-on diagnostic error occurred.  * MPU Error * TIMER Error * ROM Error * Disk Controller Error * ESDI H/W Error * RAM Diagnostic Error * Buffer Diagnostic Error (Hitachi Documentation)",
    "04,42,80,Hard Register Error at Send Diag Self Test Hard register diagnostic error was detected at Send Diag Self Test. (Hitachi Documentation)",
)


def ScanSenseData(data):
    if 1 != len(set([len(i) for i in data])):
        config.logger.critical('Malformed sense data.')
        exit(-1)
    if 4 == set([len(i) for i in data]).pop():
        pass
    elif 2 == set([len(i) for i in data]).pop():
        data = [''.join(data[i:i+2]) for i in range(0, len(data), 2)]
    else:
        config.logger.critical('Malformed sense data.')
        exit(-1)
    OP_FIELD = int(data[0][0:2], 16)
    if OP_FIELD == 0x70 or OP_FIELD == 0x71:
        K_FIELD = int(data[1][0:2], 16)
        C_FIELD = int(data[6][0:2], 16)
        Q_FIELD = int(data[6][2:4], 16)
    elif OP_FIELD == 0x72 or OP_FIELD == 0x73:
        K_FIELD = int(data[0][2:4], 16)
        C_FIELD = int(data[1][0:2], 16)
        Q_FIELD = int(data[1][2:4], 16)
    return OP_FIELD, K_FIELD, C_FIELD, Q_FIELD


def KeyDescription(K_FIELD):
    if K_FIELD == 0:
        return "NO SENSE: Indicates that there is no specific sense key information to be reported.  This may occur for a successful command or for a command that receives CHECK CONDITION status because one of the FILEMARK, EOM, or ILI bits is set to one."
    elif K_FIELD == 1:
        return "RECOVERED ERROR: Indicates that the command completed successfully, with some recovery action performed by the device server. Details may be determined by examining the additional sense bytes and the INFORMATION field. When multiple recovered errors occur during one command, the choice of which error to report - e.g., first, last, most severe -  is vendor specific."
    elif K_FIELD == 2:
        return "NOT READY: Indicates that the logical unit is not accessible. Operator intervention may be required to correct this condition."
    elif K_FIELD == 3:
        return "MEDIUM ERROR: Indicates that the command terminated with a non-recovered error condition that may have been caused by a flaw in the medium or an error in the recorded  data. This sense key may also be returned if the device server is unable to distinguish between a flaw in the medium and a specific hardware failure i.e., sense key 4h."
    elif K_FIELD == 4:
        return "HARDWARE ERROR: Indicates that the device server detected a non-recoverable hardware failure - e.g., controller failure, device failure, or parity error while performing the command or during a self test."
    elif K_FIELD == 5:
        return "ILLEGAL REQUEST: Indicates that:\n\ta. The command was addressed to an incorrect logical unit number\n\tb. The command had an invalid task attribute\n\tc. The command was addressed to a logical unit whose current configuration prohibits processing\n\tthe command\n\td. There was an illegal parameter in the CDB, or\n\te. There was an illegal parameter in the additional parameters supplied as data for some commands\n     - e.g., PERSISTENT RESERVE OUT.\nIf the device server detects an invalid parameter in the CDB, it shall terminate the command without altering the medium. If the device server detects an invalid parameter in the additional parameters supplied as data, the device server may have already altered the medium."
    elif K_FIELD == 6:
        return "UNIT ATTENTION: Indicates that a unit attention condition has been established - e.g., the removable medium may have been changed, a logical unit reset occurred."
    elif K_FIELD == 7:
        return "DATA PROTECT: Indicates that a command that reads or writes the medium was attempted on a block that is protected. The read or write operation is not performed."
    elif K_FIELD == 8:
        return "BLANK CHECK: Indicates that a write-once device or a sequential-access device encountered blank medium or format-defined end-of-data indication while reading or that a write-once device encountered a non-blank medium while writing."
    elif K_FIELD == 9:
        return "VENDOR SPECIFIC: This sense key is available for reporting vendor specific conditions."
    elif K_FIELD == 0xA:
        return "COPY ABORTED: Indicates an EXTENDED COPY command was aborted due to an error con- dition on the source device, the destination device, or both."
    elif K_FIELD == 0xB:
        return "ABORTED COMMAND: Indicates that the device server aborted the command. The application client may be able to recover by trying the command again."
    elif K_FIELD == 0xC:
        return "Obsolete"
    elif K_FIELD == 0xD:
        return "VOLUME OVERFLOW: Indicates that a buffered SCSI device has reached the end-of-partition and data may remain in the buffer that has not been written to the medium. One or more RECOVER BUFFERED DATA commands may be issued to read the unwritten data from the buffer."
    elif K_FIELD == 0xE:
        return "MISCOMPARE: Indicates that the source data did not match the data read from the medium."
    elif K_FIELD == 0xF:
        return "Reserved"
    return "Unknown"


def SenseDescription(K_FIELD, C_FIELD, Q_FIELD, sense, use_seagate):
    retval = []
    if use_seagate:
        retval.append('Seagate Interpretation:')
        if K_FIELD not in seagate_sense_codes:
            config.logger.error(
                'Sense key 0x{0:02X} is not found in Seagate sense codes.'.format(K_FIELD))
            return retval
        if C_FIELD not in seagate_sense_codes[K_FIELD]:
            config.logger.error(
                'Additional sense code 0x{1:02X} for sense key 0x{0:02X} is not found in Seagate sense codes.'.format(K_FIELD, C_FIELD))
            return retval
        if Q_FIELD not in seagate_sense_codes[K_FIELD][C_FIELD]:
            config.logger.error('Additional sense code qualifier 0x{2:02X} for sense key 0x{0:02X},0x{1:02X} is not found in Seagate sense codes.'.format(
                K_FIELD, C_FIELD, Q_FIELD))
            return retval
        offset = 4
        additional_sense_length = int(int(sense[3][2:4], 16) / 2)
        frucode = None
        while additional_sense_length:
            descriptor_type = int(sense[offset][0:2], 16)
            additional_length = int(sense[offset][2:4], 16)
            if 0x3 == descriptor_type:
                frucode = int(sense[offset + 1][2:4], 16)
            offset += 1 + int(additional_length / 2)
            additional_sense_length -= 1 + int(additional_length / 2)
        retval.append(
            'L1: ' + seagate_sense_codes[K_FIELD][C_FIELD][Q_FIELD]['L1'])
        retval.append(
            'L2: ' + seagate_sense_codes[K_FIELD][C_FIELD][Q_FIELD]['L2'])
        for fru in seagate_sense_codes[K_FIELD][C_FIELD][Q_FIELD]:
            try:
                fru = int(fru)
                if fru == frucode:
                    retval.append('FRU {0}: '.format(frucode) +
                                  seagate_sense_codes[K_FIELD][C_FIELD][Q_FIELD][fru])
            except Exception as e:
                if 'fru' == fru:
                    retval.append(
                        'FRU: ' + seagate_sense_codes[K_FIELD][C_FIELD][Q_FIELD]['fru'])
        return retval

    retval.append('HGST Interpretation:')
    if K_FIELD not in hgst_sense_codes:
        config.logger.error(
            'Sense key 0x{0:02X} is not found in HGST sense codes.'.format(K_FIELD))
        return retval
    if C_FIELD not in hgst_sense_codes[K_FIELD]:
        config.logger.error(
            'Additional sense code 0x{1:02X} for sense key 0x{0:02X} is not found in HGST sense codes.'.format(K_FIELD, C_FIELD))
        return retval
    if Q_FIELD not in hgst_sense_codes[K_FIELD][C_FIELD]:
        config.logger.error('Additional sense code qualifier 0x{2:02X} for sense key 0x{0:02X},0x{1:02X} is not found in HGST sense codes.'.format(
            K_FIELD, C_FIELD, Q_FIELD))
        return retval
    print(sense)
    offset = 4
    additional_sense_length = int(int(sense[3][2:4], 16) / 2)
    frucode = None
    while additional_sense_length:
        print(offset, sense[offset], sense)
        descriptor_type = int(sense[offset][0:2], 16)
        additional_length = int(sense[offset][2:4], 16)
        if 0x3 == descriptor_type:
            frucode = int(sense[offset + 1][2:4], 16)
        offset += 1 + int(additional_length / 2)
        additional_sense_length -= 1 + int(additional_length / 2)
    for fru in hgst_sense_codes[K_FIELD][C_FIELD][Q_FIELD]:
        try:
            fru = int(fru)
            if fru == frucode:
                retval.append('FRU {0}: '.format(frucode) +
                              hgst_sense_codes[K_FIELD][C_FIELD][Q_FIELD][fru])
        except Exception as e:
            if 'fru' == fru:
                retval.append(
                    'FRU: ' + hgst_sense_codes[K_FIELD][C_FIELD][Q_FIELD]['fru'])
    return retval

