#include <lib.h>

int main() {
  u_int s, us;
  s = get_time(&us);
  usleep(2000000);
  debugf("%d:%d\n", s, us);
  return 0;
}
