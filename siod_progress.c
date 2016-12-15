#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "siod.h"

/*
 * exit codes
 * 0  : no error
 * 1  : no SIOD running
 * 2  : lockfile error
 * 3  : shm error
 */
int main(int argc, char **argv) {
	FILE * const fp = fopen(lockfile, "r");
	if (!fp) {
		if (ENOENT == errno) {
			return 1;
		}
		fprintf(stderr, "lockfile(fopen): %s\n", strerror(errno));
		return 2;
	}
	pid_t pid = 0;
	if (1 != fscanf(fp, "%d", &pid)) {
		fprintf(stderr, "lockfile(fscanf): %s\n", strerror(errno));
		return 2;
	}
	const int shmd = shmget(pid, 0, 0);
	if (0 > shmd) {
		fprintf(stderr, "shmget: %s", strerror(errno));
		return 3;
	}
	struct shmid_ds buf;
	shmctl(shmd, IPC_STAT, &buf);
	const size_t realSize = buf.shm_segsz;
	const char * const shm_status = (char *)shmat(shmd, NULL, 0);
	if ((char *)-1 == shm_status) {
		fprintf(stderr, "shmat: %s", strerror(errno));
		return 3;
	}
	for (size_t i = 0; i < realSize; i++) {
		printf("%u ", (unsigned int)(shm_status[i]));
	}
	printf("\n");
	if (shmdt(shm_status)) {
		fprintf(stderr, "shmdt: %s", strerror(errno));
		return 3;
	}
	return 0;
}
