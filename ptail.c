/* CC0 2015 blindsay@sdf.org */

/* What's the fastest way to print from the *last* occurance of regexp
to the end of a file */

/* naïve implementation:
	back up to a newline one character at a time
	fill buffer up to previous newline
	if match
		print to eof
	reset prev newline */
/* smarter implementation?
	back up 80? chars
	read into buffer
	set cur pos to first newline
	split into whole lines
	foreach line
		if match
			print to eof */

#include <sys/types.h>
#include <sys/stat.h>

#include <assert.h>
#include <fcntl.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define GETOPT_DONE -1
#define GET_RE_ERROR_SIZE 0
#define SUCCESS 0
#define NO_COUNT 0
#define NO_COLLECT NULL
#define NO_FLAGS 0
#define FALSE 0
#define TRUE 1


regex_t *build_regex(char *, int);
off_t get_filesize(char *);
void usage(void);
void print_to_eof(FILE *, char *, char *, int);


int
main(int argc, char *argv[]) {
	int buflen, ch, matched, opt, re_flags, status;
	char *buf, *re, *file;
	off_t fsize, cur_pos, prev_eol;
	regex_t *preg;
	FILE *fp;

	re_flags = 0;

	while ((opt = getopt(argc, argv, "Ehi")) != GETOPT_DONE) {
		switch (opt) {
		case 'E':
			re_flags |= REG_EXTENDED;
			break;
		case 'i':
			re_flags |= REG_ICASE;
			break;
		default:
			usage();
			exit(-1);
		}
	}
	argc -= optind;
	argv += optind;

	if (2 != argc) {
		usage();
		exit(-1);
	}

	re = argv[0];
	preg = build_regex(re, re_flags);

	file = argv[1];
	fsize = get_filesize(file);

	fp = fopen(file, "r");

	if (NULL == fp) {
		perror(file);
		exit(-1);
	}

	prev_eol = fsize - 1;
	cur_pos = prev_eol - 1;
	buf = NULL;
	buflen = 0;
	matched = FALSE;

	while (0 <= cur_pos && !matched) {
		status = fseeko(fp, cur_pos, SEEK_SET);

		if (SUCCESS != status) {
			perror(file);
			exit(-1);
		}

		/* don't bother reading at beginning of file */
		if (0 != cur_pos) {
			ch = fgetc(fp);

			if (EOF == ch) {
				/* since loop reads backwards, EOF can only mean error */
				perror(file);
				exit(-1);
			}
		}

		if ('\n' == ch || 0 == cur_pos) {
			/* subtract one to not read the newline since regexec expects it to not
			be there, not at beginning though since we didn't do a fgetc there */
			int l = prev_eol - cur_pos - (0 != cur_pos ? 1 : 0);
			size_t read = 0;

			if (l + 1 > buflen) {
				buflen = l + 1;
				buf = realloc(buf, buflen);
				assert(NULL != buf);
			}

			/* assume position in FILE is one past newline */
			do {
				read = fread(&buf[read], sizeof(char), l - read, fp);

				if (EOF == read) {
					perror(file);
					exit(-1);
				}
			} while (read < l);

			buf[l] = '\0';

			status = regexec(preg, buf, NO_COUNT, NO_COLLECT, NO_FLAGS);

			if (SUCCESS == status) {
				printf("%s\n", buf);
				print_to_eof(fp, file, buf, buflen);
				matched = TRUE;
			}

			prev_eol = cur_pos;
		}

		cur_pos--;
	}

	return !matched;
}


void
print_to_eof(FILE *fp, char *file, char *buf, int buflen) {
	size_t read;

	fseeko(fp, ftello(fp) + 1, SEEK_SET);

	do {
		read = fread(buf, sizeof(char), buflen - 1, fp);

		if (EOF == read && ferror(fp)) {
			perror(file);
			exit(-1);
		}
		else {
			buf[read] = '\0';
			printf("%s", buf);
		}
	} while (!feof(fp));
}


regex_t *
build_regex(char *re, int extra_flags) {
	regex_t *preg;
	int re_flags = REG_NOSUB | extra_flags;
	int status;

	preg = malloc(sizeof(regex_t));
	assert(NULL != preg);

	status = regcomp(preg, re, re_flags);

	if (SUCCESS != status) {
		size_t len = regerror(status, preg, NULL, GET_RE_ERROR_SIZE);
		char *msg = malloc(len);
		assert(NULL != msg);

		regerror(status, preg, msg, len);

		fprintf(stderr, "Invalid regular expression: %s\n", msg);

		exit(-1);
	}

	return preg;
}


off_t
get_filesize(char *file) {
	int status;
	off_t size;
	struct stat *sb = malloc(sizeof(struct stat));

	assert(NULL != sb);

	status = stat(file, sb);

	if (SUCCESS != status) {
		perror(file);

		exit(-1);
	}

	size = sb->st_size;

	free(sb);

	return size;
}


void
usage(void) {
	printf("ptail [-Ei] pattern file\n\t-E extented regex\n\t-i Case insensitive\n");
}
