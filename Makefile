CFLAGS = -g -Wall -Wextra -ansi -pedantic

all: check_c_format find_high_char mkdirp space2tab

clean:
	$(RM) check_c_format.o find_high_char.o mkdirp.o space2tab.o

fclean: clean
	$(RM) check_c_format find_high_char mkdirp space2tab

check_c_format: check_c_format.o
	$(CC) -o $@ $(LDFLAGS) $^ $(LDLIBS)

find_high_char: find_high_char.o
	$(CC) -o $@ $(LDFLAGS) $^ $(LDLIBS)

mkdirp: mkdirp.o
	$(CC) -o $@ $(LDFLAGS) $^ $(LDLIBS)

space2tab: space2tab.o
	$(CC) -o $@ $(LDFLAGS) $^ $(LDLIBS)

.PHONY: all clean fclean
