#include <lib.h>

int main() {
  barrier_alloc(11);
  for (int i = 0; i < 9; i++) {
    int who = fork();
    if (who == 0) {
      debugf("I'm son!\n");
      barrier_wait();
      syscall_panic("Wrong block!");
    }
  }
  barrier_wait();
  debugf("I'm finished!\n");
  syscall_panic("G");
  return 0;
}
