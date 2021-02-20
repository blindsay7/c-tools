/* CC0 2015 blindsay@sdf.org */

/* How can I show the individual differences between two strings? */

#include <stdio.h>
#include <string.h>

int
main(int argc, char *argv[])
{
	size_t i, l1, l2, lmax, lmin;

	if (3 != argc) {
		return 1;
	}

	l1 = strlen(argv[1]);
	l2 = strlen(argv[2]);
	lmax = (l1 >= l2 ? l1 : l2);
	lmin = (l1 <= l2 ? l1 : l2);

	printf("%s\n", argv[1]);

	for (i = 0; i < lmax; i++) {
		if (i > lmin || argv[1][i] != argv[2][i]) {
			printf("|");
		} else {
			printf(" ");
		}
	}

	printf("\n%s\n", argv[2]);

	return 0;
}
