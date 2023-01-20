#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define UNIX_MODE 1
#define DOS_MODE 2

void find_high(int fd, const char *filename)
{
	unsigned char buf[8192];
	ssize_t r;
	unsigned long line_no = 1;
	unsigned long col_no = 1;
	unsigned char last_char = '\0';
	int mode = 0;

	while ((r = read(fd, buf, sizeof (buf))) > 0) {
		ssize_t i;
		for (i = 0; i < r; ++i) {
			if (buf[i] == '\n') {
				mode |= last_char == '\r' ? DOS_MODE : UNIX_MODE;
				col_no = 0;
				++line_no;
			} else {
				if (last_char == '\r') {
					printf("CR without LF at %s:%lu,%lu\n", filename, line_no, col_no - 1);
				}
				if (buf[i] & 0x80) {
					printf("high char 0x%02x at %s:%lu,%lu\n", buf[i], filename, line_no, col_no);
				} else if ((buf[i] == 0x7f || buf[i] < 0x20) &&
				           buf[i] != '\r' && buf[i] != '\t') {
					printf("control char 0x%02x at %s:%lu,%lu\n", buf[i], filename, line_no, col_no);
				}
			}
			++col_no;
			last_char = buf[i];
		}
	}
	if (last_char == '\r') {
		printf("CR without LF at %s:%lu,%lu\n", filename, line_no, col_no - 1);
	}
	if (r < 0) {
		perror(filename);
	}
	if (mode & DOS_MODE) {
		if (mode & UNIX_MODE) {
			printf("%s: DOS+UNIX mode", filename);
		} else {
			printf("%s: DOS mode", filename);
		}
	} else {
		if (mode & UNIX_MODE) {
			printf("%s: UNIX mode", filename);
		} else {
			printf("%s: no LF", filename);
		}
	}
	if (last_char == '\n') {
		printf("\n");
	} else {
		printf(" / no newline at EOL \n");
	}
}

int main(int argc, char *argv[])
{
	if (argc == 1) {
		find_high(0, "<stdin>");
	} else {
		char * const *p = argv + 1;
		for (; *p; ++p) {
			int fd = open(*p, O_RDONLY, 0777);
			if (fd < 0) {
				perror(*p);
			} else {
				find_high(fd, *p);
				if (close(fd) != 0) {
					perror(*p);
				}
			}
		}
	}
	return 0;
}
