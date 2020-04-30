The simple I/O driver is used to either read or write a drive. It has a
minimalist design. It touchs every block on the drive exactly once. All I/Os
have the same size. The queue depth is held constant. It has the following data
integrity model:
1> When writing, data in a block is a hash of the block address and a key (a
command line parameter)
2> When reading a block, it reads the actual data, detects the key inside it
and then generates the expected data and compares it with actual.

It logs its progress and errors in a log file.

This program can be used as a building block for a larger project that tests
drive quality.

Usage:

./siod operation locality I/O_size queue_depth encoding logfile_prefix> device+

	Operation   - [rw] read/write
	Locality    - [rs] random/sequential
	I/O size    - [0-9]+ number of blocks
	Queue depth - [0-9]+ 
	Encoding    - [0123]
			0 - Write -> all zeroes      Read -> detect encoding and check data (all zeros is invalid data)
			1 - Write -> with encoding 1 Read -> check for encoding 1
			2 - Write -> with encoding 2 Read -> check for encoding 2
			3 - Write -> with encoding 3 Read -> check for encoding 3
	Device      - <sg>:<ha>:<ta>:<ts>
			sg - [0-9]+ The sg device number, device XXX refers to /dev/sgXXX
			ha - [0-9a-fA-F]+ The host address as an 8 byte hexadecimal integer
			ta - [0-9a-fA-F]+ The target address as an 8 byte hexadecimal integer
			ts - [0-9]+ The time stamp as an 8 byte integer
