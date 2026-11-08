#include <stdio.h>
#include <time.h>
#include <unistd.h>

/*
// escape stuff:
\033[A -	up 1 line
\033[B -	down 1 line
\033[C -	right 1 col
\033[D -	left 1 col
\033[H -	move to top-left of screen
\033[<row>;<col>H -	move to specific position
\033[K -	clear from cursor to end of line
\033[2K -	clear entire line
\033[J -	clear from cursor to end of screen
\033[2J -	clear entire screen
\033[s / \033[u -	save / restore cursor position
\033 - is ESC (octal 033 = 0x1B).

// char stuff:
\n - new line
\r - return to start of the line
\t - simple tab
*/

int main() {

	printf("Digital CLock\n");

	time_t rawtime = 0; // time since Unix Epoch (1970 Jan 1)
	struct tm *pTime = NULL;

	while (1) {
		time(&rawtime);

		pTime = localtime(&rawtime);

		//printf("\r%lld", time); // primitive print of seconds
		printf("\r\033[K%02d:%02d:%02d", pTime->tm_hour, pTime->tm_min, pTime-> tm_sec);
		fflush(stdout);
		
		sleep(1);
	}

}
