/* CC0 2015 blindsay@sdf.org */

/*
Quick and dirty test to answer the question:
Is there anything faster than `find . -type f | wc -l`
*/

#include <sys/stat.h>
#include <sys/types.h>
#include <assert.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define GETOPT_DONE -1
#define MATCH 0
#define QUIET (1 << 0)
#define RECURSE (1 << 1)
#define SUCCESS 0

struct dirstack {
	char *path;
	struct dirstack *next;
};
typedef struct dirstack dirstack;

unsigned int dircount(const char *, int);
void usage(const char *);
dirstack *pushdir(dirstack *, const char *);
dirstack *popdir(dirstack *);

int
main(int argc, char *argv[]) {
	unsigned int cnt = 0;
	int flags = 0, opt;
	char *progname = argv[0];

	while ((opt = getopt(argc, argv, "hqr")) != GETOPT_DONE)
                switch (opt) {
                case 'q':
                        flags |= QUIET;
                        break;
                case 'r':
                        flags |= RECURSE;
                        break;
                default:
                        usage(progname);
			/* NOTREACHED */
                }
        argc -= optind;
        argv += optind;

	if (1 != argc)
		usage(progname);

	cnt = dircount(argv[0], flags);

	if (RECURSE & flags)
		printf("--------\nTotal\t%u\n", cnt);

	return 0;
}

unsigned int
dircount(const char *n, int flags) {
	dirstack *stack = pushdir(NULL, n);
	unsigned int total = 0;

	while (NULL != stack) {
		DIR *dir = opendir(stack->path);
		char cwd[MAXNAMLEN + 1];
		unsigned int dircnt = 0;
		struct dirent *ent;

		if (NULL == dir) {
			perror(stack->path);
			exit(1);
		}

		strcpy(cwd, stack->path);

		stack = popdir(stack);

		while (NULL != (ent = readdir(dir))) {
			char path[MAXNAMLEN + 1];
			struct stat s;
			int res;

			if (MATCH == strcmp(".", ent->d_name)
				    || MATCH == strcmp("..", ent->d_name))
				continue;

			strcpy(path, cwd);
			strcat(path, "/");
			strcat(path, ent->d_name);

			/* dirent->d_type was always set to DT_UNKNOWN on SDF */
			res = lstat(path, &s);

			if (SUCCESS != res)
				perror(path);

			switch (s.st_mode & S_IFMT) {
			case S_IFDIR:
				if (RECURSE & flags)
					stack = pushdir(stack, path);
				break;
			case S_IFREG:
				total++;
				dircnt++;
				break;
			}
		}

		if (!(RECURSE & flags) || !(QUIET & flags)) 
			printf("%s\t%u\n", cwd, dircnt);

		closedir(dir);
	}

	return total;
}

void
usage(const char *progname) {
	printf("usage: %s [-h] [-r [-q]] directory\n%s%s",
		progname,
		"\t-r\tRecurse\n",
		"\t-q\tQuiet. Only print recursive total\n"
	);

	exit(-1);
}

dirstack *
pushdir(dirstack *stack, const char *dir) {
	dirstack *new = malloc(sizeof(dirstack));
	assert(NULL != new);

	new->path = malloc(strnlen(dir, MAXNAMLEN) + 1);
	assert(NULL != new->path);

	strcpy(new->path, dir);
	new->next = stack;

	return new;
}

dirstack *
popdir(dirstack *stack) {
	dirstack *next = stack->next;

	free(stack->path);
	free(stack);

	return next;
}
