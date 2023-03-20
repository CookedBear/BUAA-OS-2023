#include <stdio.h>

int main() {
  long l1 = -1;
  printf("%d", (int)(l1 >> 63) && 1);
  return 0;
}
