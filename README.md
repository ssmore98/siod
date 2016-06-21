The simple I/O driver is used to either read or write a drive. It has a
minimalist design. It touchs every block on the drive exactly once. All I/Os
have the same size. The queue depth is held constant. It has the following data
integrity model:
1> When writing, data in a block is a hash of the block address and a key (a
command line parameter)
2> When reading a block, it reads the actual data, detects the key inside it
and then generates the expected data and compares it with actual.

It logs its progress and errors in a log file.

This program will be used as a building block for a larger project that will
test drive quality.