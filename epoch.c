/* CC0 2015 blindsay@sdf.org */

/* Print current epoch seconds with micro seconds */

#include <sys/time.h>
#include <stdio.h>

#define GETTIME_ERROR -1

int
main(void) {
	int ret;
	struct timeval tv;

	ret = gettimeofday(&tv, NULL);

	if (GETTIME_ERROR == ret) {
		perror(NULL);
		return -1;
	}

	printf("%ld.%06ld\n", tv.tv_sec, tv.tv_usec);

	return 0;
}
