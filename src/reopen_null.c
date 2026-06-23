#include <unistd.h>
#include <fcntl.h>

int reopen_null(int fd) {
  int dn, r =-1;

  dn = open("/dev/null", O_RDWR);
  if (dn >= 0) {
    r = dup2(dn, fd);
    close(dn);
  }

  return r;
}
