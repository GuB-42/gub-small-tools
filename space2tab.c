#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

void convert(int fd_in, const char *filename)
{
	unsigned char buf[8192];
	unsigned long line_no = 1;
	unsigned long col_no = 1;

	int all_space = 1;
	unsigned char last_c = '\0';
	unsigned char last_c_nospace = '\0';

	unsigned char *iline = 0;
	size_t iline_length = 0;
	size_t iline_max_size = 0;

	unsigned *tab_levels = 0;
	size_t tab_level_count = 0;
	size_t tab_level_max = 0;

	unsigned cur_tab_level = 0;
	unsigned cur_tab_offset = 0;

	ssize_t r;
	while ((r = read(fd_in, buf, sizeof (buf))) > 0) {
		ssize_t i;
		for (i = 0; i < r; ++i) {
			unsigned char c = buf[i];

			if (col_no >= iline_max_size) {
				unsigned char *rbuf;
				size_t s = iline_max_size < 512 ? 512 : (iline_max_size << 1);
				rbuf = (unsigned char *)realloc(iline, s);
				if (rbuf) {
					iline = rbuf;
					iline_max_size = s;
				}
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
					fprintf(stderr,"EOL spaces at %s:%lu\n", filename, line_no);
				}
				++line_no;
				col_no = 0;
				all_space = 1;
				cur_tab_level = 0;
				cur_tab_offset = 0;
				putchar(c);
			} else if (c == '\t') {
				if (all_space) {
					if (tab_levels && tab_level_count > 0) {
						if (cur_tab_level < tab_level_count) {
							cur_tab_offset = 0;
							++cur_tab_level;
						} else {
							cur_tab_offset = tab_levels[tab_level_count - 1];
						}
					} else {
						cur_tab_offset = 1;
					}
				} else {
					putchar(' ');
				}
			} else if (c == ' ') {
				if (all_space) {
					++cur_tab_offset;
					if (tab_levels && cur_tab_level < tab_level_count) {
						if (cur_tab_offset >= tab_levels[cur_tab_level]) {
							cur_tab_offset = 0;
							++cur_tab_level;
						}
					}
				} else {
					putchar(c);
				}
			} else {
				if (all_space) {
					unsigned j;
					int tabulate = (cur_tab_offset > 0 &&
					                cur_tab_level >= tab_level_count);
					if (tabulate) {
						tabulate =
							(last_c_nospace == '{' ||
							 col_no - 2 >= iline_length || iline[col_no - 2] != '(');
					}
					if (tabulate) {
						if (cur_tab_level >= tab_level_max) {
							unsigned *rbuf;
							size_t s = tab_level_max < 32 ? 32 : tab_level_max << 1;
							rbuf = (unsigned *)realloc(tab_levels, s * sizeof (unsigned));
							if (rbuf) {
								tab_levels = rbuf;
								tab_level_max = s;
							}
						}
						if (cur_tab_level < tab_level_max) {
							tab_levels[cur_tab_level] = cur_tab_offset;
							cur_tab_offset = 0;
							++cur_tab_level;
						}
					}
					if (cur_tab_level > 0 || c == '}') {
						tab_level_count = cur_tab_level;
					}
					for (j = 0; j < cur_tab_level; ++j) {
						putchar('\t');
					}
					for (j = 0; j < cur_tab_offset; ++j) {
						putchar(' ');
					}
					all_space = 0;
				}
				putchar(c);
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
		putchar('\n');
	}

	free(iline);
	free(tab_levels);
	iline = 0;
}

int main(int argc, char *argv[])
{
	if (argc == 1) {
		convert(STDIN_FILENO, "<stdin>");
	} else {
		char * const *p = argv + 1;
		for (; *p; ++p) {
			int fd = open(*p, O_RDONLY, 0777);
			if (fd < 0) {
				perror(*p);
			} else {
				convert(fd, *p);
				if (close(fd) != 0) {
					perror(*p);
				}
			}
		}
	}
	return 0;
}
