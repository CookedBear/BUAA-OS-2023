// user/shmtest.c
#include "lib.h"
void main()
{
    volatile u_int *a = (volatile u_int *)0x23333334;
    make_shared((void *)a);
    *a = 233;
    debugf("arrive");
    if (fork() == 0)
    {
        u_int ch = syscall_getenvid();
        *a = ch;
        while (*a == ch)
            syscall_yield();
        debugf("i am %x\nparent is %x\n", ch, *a);
    }
    else
    {
        u_int ch = syscall_getenvid();
        while (*a == 233)
            syscall_yield();
        debugf("i am %x\nchild is %x\n", ch, *a);
        *a = syscall_getenvid();
    }
}