/* http://julipedia.meroh.net/2004/10/example-of-kqueue.html */

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


int
main(int argc, char *argv[]) {
	int f;
	int kq;
	int nev;
	struct kevent change;
	struct kevent event;

	kq = kqueue();

	if (-1 == kq) {
		perror("kqueue");
	}

	f = open(argv[1], O_RDONLY);

	if (-1 == f) {
		perror("open");
	}

	EV_SET(&change, f, EVFILT_VNODE, EV_ADD | EV_ENABLE | EV_ONESHOT,
	    NOTE_WRITE, 0, 0);

	nev = kevent(kq, &change, 1, &event, 1, NULL);

	if (-1 == nev) {
		perror("kevent");
	}
	else if (0 < nev) {
		if (event.fflags & NOTE_EXTEND ||
		    event.fflags & NOTE_WRITE) {
			printf("File modified\n");
		}
	}

	close(kq);
	close(f);

	return EXIT_SUCCESS;
}
