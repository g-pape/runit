#define _GNU_SOURCE
#include <sched.h>

int main(void) {
  return unshare(0);
}
