#include <asm/asm.h>
#include <env.h>
#include <kclock.h>
#include <pmap.h>
#include <printk.h>
#include <trap.h>

// When build with 'make test lab=?_?', we will replace your 'mips_init' with a
// generated one from 'tests/lab?_?'.
#ifdef MOS_INIT_OVERRIDDEN
#include <generated/init_override.h>
#else

void mips_init() {
  int i1 = 1;
  int i2 = -1;
  long l1 = -1;
  long l2 = 1222222222;
  printk("init.c:\tmips_init() is called\n");
  printk("you have %05R\n", i1, i2);
  printk("you have %-5R\n", 1, 20);
  printk("you have %05lR\n", l1, l2);
  printk("you have %-5lR\n", l1, l2);
  printk("you have %lR\n", l1, l2);
  printk("%ld", l1);

  // lab2:
  // mips_detect_memory();
  // mips_vm_init();
  // page_init();

  // lab3:
  // env_init();

  // lab3:
  // ENV_CREATE_PRIORITY(user_bare_loop, 1);
  // ENV_CREATE_PRIORITY(user_bare_loop, 2);

  // lab4:
  // ENV_CREATE(user_tltest);
  // ENV_CREATE(user_fktest);
  // ENV_CREATE(user_pingpong);

  // lab6:
  // ENV_CREATE(user_icode);  // This must be the first env!

  // lab5:
  // ENV_CREATE(user_fstest);
  // ENV_CREATE(fs_serv);  // This must be the second env!
  // ENV_CREATE(user_devtst);

  // lab3:
  // kclock_init();
  // enable_irq();
  while (1) {
  }
}

#endif
