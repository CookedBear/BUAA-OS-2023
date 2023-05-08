#include <lib.h>

static void os_assert(int cond, const char *err) {
  if (!cond) {
    user_halt("%s\n", err);
  }
}

int main() {
  int items_id = sem_init("items", 5, 0);
  debugf("init\n");
  int lock_id = sem_init("lock", 0, 0);
  int r = fork();
  if (r < 0) {
    user_halt("OSTEST_FORK");
  }
  if (r == 0) {
    debugf("son\n");
    sem_wait(lock_id);
    os_assert(sem_getvalue(items_id) == 4, "WRONG_RETURN_VALUE_WAIT1");
    os_assert(sem_getvalue(lock_id) == 0, "WRONG_RETURN_VALUE_WAIT2");
    debugf("OSTEST_OK\n");
    return 0;
  } else {
    debugf("fether %d\n", items_id);
    os_assert(sem_getvalue(items_id) == 5, "WRONG_RETURN_VALUE1");
    os_assert(sem_getvalue(lock_id) == 0, "WRONG_RETURN_VALUE2");
    debugf("value and wait\n");

    sem_wait(items_id);
    debugf("out wait\n");

    os_assert(sem_getvalue(items_id) == 4, "WRONG_RETURN_VALUE_WAIT1");
    os_assert(sem_getvalue(lock_id) == 0, "WRONG_RETURN_VALUE_WAIT2");
    debugf("before lock post\n");

    sem_post(lock_id);
    debugf("OSTEST_OK\n");
    return 0;
  }
}
