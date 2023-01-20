#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

int mkdirp(const char *pathname, mode_t mode)
{
  char          *buf;
  unsigned      len;
  int           ret = 0;
  char          *p, *last;

  len = strlen(pathname);
  if (len == 0) return mkdir("", mode);
  buf = (char *)malloc(len + 1);
  if (!buf) return -1;
  memcpy(buf, pathname, len);

  last = buf + len - 1;
  while (*last == '/' && last != buf) --last;
  *++last = '\0';

  while (1) {
    ret = mkdir(buf, mode);
    if (ret == 0) break;
    if (errno != ENOENT) goto mkdirp_end;
    p = strrchr(buf, '/');
    if (!p) goto mkdirp_end;
    do {
      if (p == buf) goto mkdirp_end;
      *p-- = '\0';
    } while (*p == '/');
  }

  p = buf + strlen(buf);
  while (1) {
    while (*p == '\0') {
      if (p == last) goto mkdirp_end;
      *p++ = '/';
    }
    ret = mkdir(buf, mode);
    if (ret != 0) goto mkdirp_end;
    p += strlen(p);
  }

 mkdirp_end:
  free(buf);
  return ret;
}

#include <stdio.h>

int main(int argc, char **argv)
{
  int   ret;

  if (argc != 2) return 1;
  ret = mkdirp(argv[1], 0777);
  if (ret != 0) perror("mkdirp");
  return ret;
}
