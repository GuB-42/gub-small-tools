#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

void find_high_and_tab(int fd, const char *filename)
{
	unsigned char buf[8192];
	unsigned long line_no = 1;
	unsigned long col_no = 1;

	unsigned prev_start_tabs = 0;
	unsigned start_tabs = 0;
	int all_tabs = 1;
	int all_space = 1;
	unsigned char last_c = '\0';
	unsigned char last_c_nospace = '\0';

	unsigned char *iline = 0;
	size_t iline_length = 0;
	size_t iline_max_size = 0;

	ssize_t r;
	while ((r = read(fd, buf, sizeof (buf))) > 0) {
		ssize_t i;
		for (i = 0; i < r; ++i) {
			unsigned char c = buf[i];

			if (col_no >= iline_max_size) {
				unsigned char *rbuf;
				if (iline_max_size < 512)  {
					iline_max_size = 512;
				} else {
					iline_max_size <<= 1;
				}
				rbuf = (unsigned char *)realloc(iline, iline_max_size);
				if (rbuf) iline = rbuf;
			}
			if (col_no < iline_max_size) {
				if (c == '\n') {
					iline_length = col_no - 1;
					iline[iline_length] = '\0';
				} else if ((col_no - 1) >= iline_length) {
					iline[iline_length++] = c;
					iline[iline_length] = '\0';
				} else if (c != ' ') {
					iline[col_no - 1] = c;
				}
			}

			if (c == '\n') {
				if (last_c == ' ' || last_c == '\t') {
					printf("EOL spaces at %s:%lu\n", filename, line_no);
				}
				++line_no;
				col_no = 0;
				prev_start_tabs = start_tabs;
				start_tabs = 0;
				all_tabs = 1;
				all_space = 1;
			} else if (c == '\t') {
				if (all_tabs) {
					++start_tabs;
				} else {
					printf("bad tab at %s:%lu,%lu\n",
					       filename, line_no, col_no);
				}
			} else if (c & 0x80 || buf[i] == 0x7f || c < 0x20) {
				printf("char 0x%02x at %s:%lu,%lu\n",
				       c, filename, line_no, col_no);
			} else if (c == ' ') {
				if (all_tabs) {
					if (prev_start_tabs != start_tabs) {
						printf("tab-space mix at %s:%lu,%lu\n",
						       filename, line_no, col_no);
					}
					all_tabs = 0;
				}
			} else {
				all_tabs = 0;
				if (all_space) {
					if (last_c == ' ' &&
					    (last_c_nospace == '{' ||
					     (!(col_no - 2 < iline_length &&
					        iline[col_no - 2] == '(') &&
					     (!(col_no - 1 < iline_length &&
					        iline[col_no - 2] == '/' &&
					        iline[col_no - 1] == '*'))))) {
						printf("space indent (T1) at %s:%lu,%lu\n",
						       filename, line_no, col_no);
					}
					all_space = 0;
				}
				last_c_nospace = c;
			}
			++col_no;
			last_c = c;
		}
	}
	if (r < 0) {
		perror(filename);
	}
	if (last_c != '\n') {
		printf("no EOL at end of file %s\n", filename);
	}

	free(iline);
	iline = 0;
}

int main(int argc, char *argv[])
{
	if (argc == 1) {
		find_high_and_tab(0, "<stdin>");
	} else {
		char * const *p = argv + 1;
		for (; *p; ++p) {
			int fd = open(*p, O_RDONLY, 0777);
			if (fd < 0) {
				perror(*p);
			} else {
				find_high_and_tab(fd, *p);
				if (close(fd) != 0) {
					perror(*p);
				}
			}
		}
	}
	return 0;
}
