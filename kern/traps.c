#include <env.h>
#include <pmap.h>
#include <printk.h>
#include <trap.h>

extern void handle_int(void);
extern void handle_tlb(void);
extern void handle_sys(void);
extern void handle_mod(void);
extern void handle_ov(void);
extern void handle_reserved(void);

void (*exception_handlers[32])(void) = {
    [0 ... 31] = handle_reserved,
    [0] = handle_int,
    [2 ... 3] = handle_tlb,
    [12] = handle_ov,
#if !defined(LAB) || LAB >= 4
    [1] = handle_mod,
    [8] = handle_sys,
#endif
};

/* Overview:
 *   The fallback handler when an unknown exception code is encountered.
 *   'genex.S' wraps this function in 'handle_reserved'.
 */
void do_reserved(struct Trapframe *tf) {
  print_tf(tf);
  panic("Unknown ExcCode %2d", (tf->cp0_cause >> 2) & 0x1f);
}

void do_ov(struct Trapframe *tf) {
  // printk("Got an ov\n");
  u_int badva = tf->cp0_epc;
  Pde *pgdir = curenv->env_pgdir;
  Pte **ppte = NULL;
  struct Page *p = page_lookup(pgdir, badva, ppte);
  // pgdir_walk(pgdir, badva, 0, &p);
  u_int badpa = page2pa(p) + (badva & 0xfff);
  u_int *badk0va = 0x80000000 | badpa;
  u_int code = *badk0va;
  // printk("got code %x\n", code);
  if (code & 0x20000000) { // addi
    u_int s = (code & 0x3e00000) >> 21;
    u_int t = (code & 0x1f0000) >> 16;
    u_int svalue = tf->regs[s];
    u_int imm = code & 0xffff;
    tf->regs[t] = svalue / 2 + imm / 2;
    tf->cp0_epc += 4;
    // printk("inform:%x %d %d %x %x\n", code, s, t, svalue, tf->regs[t]);
    printk("addi ov handled\n");
  } else if ((code & 0x1f) == 0) { // add
    printk("add ov handled\n");
    *badk0va = code + 1;
  } else { // sub
    printk("sub ov handled\n");
    *badk0va = code + 1;
  }
  curenv->env_ov_cnt++;
}
