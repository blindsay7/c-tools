/* CC0 2015 blindsay@sdf.org */

/*
Print current epoch seconds with micro seconds, or convert ISO8601
date-time to epoch seconds
*/

#include <sys/time.h>
#include <stdio.h>
#include <time.h>

#define GETTIME_ERROR -1
#define STRFTIME_ERROR 0

int
main(int argc, char *argv[]){
	if (1 < argc){
		char *strpret,str[24];
		size_t strfret;
		struct tm tm;

		strpret=strptime(argv[1],"%FT%T", &tm);

		if (NULL==strpret){
			strpret=strptime(argv[1],"%F", &tm);

			if (NULL==strpret){
				fprintf(stderr,"Invalid date provided: %s",argv[1]);
				return -1;
			}
		}

		strfret=strftime(str,23,"%s",&tm);

		if(STRFTIME_ERROR==strfret){
			fprintf(stderr,"Error formatting time for %s",argv[1]);
			return -1;
		}

		printf("%s\n",str);
	}else{
		int ret;
		struct timeval tv;

		ret=gettimeofday(&tv,NULL);

		if(GETTIME_ERROR==ret){
			perror(NULL);
			return -1;
		}

		printf("%ld.%06ld\n",tv.tv_sec,tv.tv_usec);
	}

	return 0;
}
