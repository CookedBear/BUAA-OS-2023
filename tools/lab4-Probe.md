# Lab4 - 系统调用与进程创建

Lab4 中主要涉及到以下内容：

- 系统调用 `syscall` 的流程
- 进程间通信机制：ipc
- **用户**进程的创建方式：`fork` 函数
- 写时复制 **COW** 与页写入异常

> 首先需要明确一件事，这篇文里说的大部分和汇编相关的内容估计都不会考，只是用来全面理解每一步的具体作用而已。
>
> 其实最不好理解的部分是在汇编代码和C代码之间切换的部分，可能一些宏定义、编译器的一些操作就会漏掉很多细节。
>
> 所以，时不时地用用 `make objdump` 吧！努力成为 buaa-os 领域大神（？

# 系统调用

在计算机组成原理实验 P7 与 Lab3 中，我们大致已经能够理清异常的原因、怎样产生与处理的方式了，但在 MOS 之前提到的异常大多不能由用户主动触发（时钟中断、TLB MISS 等），接下来我们就要深入了解一种特殊的、可以**由用户主动发起的异常**：系统调用 `syscall`

- 异常发生的原因：出现某些不符合操作规范的指令、外部中断、syscall 指令
- 产生地点（硬件）：指令流水至（M级）时 CP0 协处理器分析异常，修改 SR、Cause 等寄存器，使得当前 PC 跳转至异常处理程序入口（`0x80000000`）
- 异常处理（软件）：异常处理程序对 CP0 记录的信息进行分析，选择对应的 handler 函数并跳转；对应 handler 处理结束后通过 ret_from_exception 汇编函数返回 EPC，程序继续执行

我们想要使用 syscall 指令，为的就是**进入内核态**，让操作系统为用户执行某些任务，这些任务出于安全等考虑，只有内核才能完成，所以在调用 syscall 时，就不可避免地要进行 CPU 状态的切换（标记于 SR 寄存器的 KUc 位中）

## 系统调用是干啥（）

在详细了解系统调用前，还有必要再补充一下进程运行时的虚拟空间知识（**可以补充在Lab3中？**）

每个进程，都有相同的虚拟地址划分方式，并按照 mmu.h 文件中所示进行排布。每个进程都具有一张这样的表。同时每个进程的 kseg0、kseg1 段也都存放（或者说映射）着内核相关的数据结构，存在于所有进程的虚拟空间中，相当于被所有进程**只读共享**。所以为了方便使用，它们被整体映射到物理地址的固定区域；对于 kuseg 段，用户的页表和虚拟地址相结合，会指向物理内存中的某些空间，不同进程可能对物理空间进行共享。

执行系统调用，就是从 kuseg 段的指令跳转至 kseg0 段，（进入内核态）并执行特定序列（系统调用函数），最后返回用户态 EPC 。

## 系统调用过程概览

我觉得 os 学的就是一个全局观，先把需求和路线理清了，细节咱们随后再细说（

- 用户提出请求：`syscall_` 函数

在 MOS 中，操作系统为用户准备了一系列可以在用户态调用的函数： `syscall_*` 系列函数，他们每个函数都对应一个可以通过系统调用完成的任务，用户通过调用这些函数，向操作系统传达信息：**我要使用系统调用完成某个功能**

- 进入内核态：汇编函数 `msyscall`

在刚提到的 `syscall_*` 函数中，存在一个 `msyscall` 函数，在这个汇编写成的 `msyscall` 中，才出现了真正的 `syscall` 汇编指令，也就是在这里正式地进入了内核态，准备使用异常处理程序解决 `syscall`

- 接收请求并分类：`handle_sys` （`do_syscall`）

在异常处理程序中，调用 SAVE_ALL 保存当前现场为内核栈中的 trapframe ，并将当前使用栈转换为内核栈。通过异常处理程序判断为系统调用后，转入 handler ，即 `do_syscall` 函数，在这个函数中，我们通过分析用户传入的信息（`syscall_*` 的类型和用户现场）来响应系统调用

- 响应完毕，返回用户态：`ret_from_exception`

在从 `do_syscall` 返回至 `handle_sys` 后，最后会和其他异常一样，执行 `ret_from_exception`，还原现场，返回用户态，整个系统调用的过程结束



至此，整个执行流程可以总结为这张图片：

![image-20230423162717568](https://cookedbear-2003-1307884465.cos.ap-beijing.myqcloud.com/NotePics/202304231627668.png)

## 发起系统调用 - syscall_*

上面提到，可由用户调用、距离内核态最近的函数就是这一系列 `syscall_*` 函数了。它们作为用户可调用的函数，位于 `user/lib/syscall_lib.c` 文件中，现在来看看它们的具体内容：

```c
// 为节省空间仅保留了部分函数，反正所有函数都长这样()
void syscall_putchar(int ch) {
	msyscall(SYS_putchar, ch);
}

u_int syscall_getenvid(void) {
	return msyscall(SYS_getenvid);
}

int syscall_mem_map(u_int srcid, void *srcva, u_int dstid, void *dstva, u_int perm) {
	return msyscall(SYS_mem_map, srcid, srcva, dstid, dstva, perm);
}
```

可以看到，它们都只调用了**不同参数**的 `msyscall` 函数，然后早早跑路（x）。这里还需要注意，`syscall_putchar`、`syscall_yield`、`syscall_panic` 这几个函数没有以”return“的方式调用 `msyscall` ，因为他们不是 `void` 的，就是 `noreturn` 的，憋憋

可以发现每个不同的函数，第一个参数一定不同，并且都代表了这个函数。也就是说， `msyscall` 通过接收这系列函数传入的第一个参数，决定最后响应的内核函数是谁，而后续的参数充当信息，用于辅助处理。



## 转入内核态 - msyscall - Exercise 4.1

这个在用户态执行的最后一个函数（但用户编程过程中实际上不用），位于 `user/lib/syscall_wrap.S` 中，这个函数很简单：

```assembly
#include <asm/asm.h>

LEAF(msyscall)
    // Just use 'syscall' instruction and return.
	
    /* Exercise 4.1: Your code here. */
    syscall
    jr ra
END(msyscall)

```

这个函数实际上充当了用户态、内核态的转接口：执行 `syscall` 进入内核态，从调用返回后执行 `jr ra` ，十分简洁，分工明确

我们可能注意到了不同的 `msyscall` 调用可能有不同的参数数量，他们都被保存在堆栈中为函数创造的 stack frame 空间中，与 sp 相邻。

下一步，内核就会接收到由硬件产生的 8 号异常，通过处在 `kern/entry.S` 的异常分发程序 `exc_gen_entry` 跳转到 handler 函数： `do_syscall`



## 分发系统调用 - do_syscall

首先需要注意，在跳转至 `do_syscall` 前，我们在异常分发程序中向内核栈（SAVE_ALL）压入了用户态 trapframe 的信息。随后又通过 move 指令把 a0 寄存器复制成了 trapframe 的地址（通过 move sp 的值）

所以 `do_syscall` 在调用时就会自动地带有一个参数，它就是存放在 a0 寄存器中的用户态 `trackframe` **指针**（为什么是指针？因为传入的 sp 的值实际上指向了存放 tf 的地址）



```c
void do_syscall(struct Trapframe *tf) {
	int (*func)(u_int, u_int, u_int, u_int, u_int);
	int sysno = tf->regs[4];    // sysno 是 msyscall 的第一个参数
	if (sysno < 0 || sysno >= MAX_SYSNO) {
		tf->regs[2] = -E_NO_SYS;
		return;
	}

	/* Step 1: 移动 EPC，使得syscall返回后执行下一条指令 */
	/* Exercise 4.2: Your code here. (1/4) */
	tf->cp0_epc += 4;
	/* Step 2: 通过 sysno 获得使用的处理函数 */
	/* Exercise 4.2: Your code here. (2/4) */
	func = syscall_table[sysno];
	/* Step 3: 获取前三个参数 $a1, $a2, $a3. */
	u_int arg1 = tf->regs[5];
	u_int arg2 = tf->regs[6];
	u_int arg3 = tf->regs[7];

	/* Step 4: 获取后两个参数 [$sp + 16 bytes], [$sp + 20 bytes] */
	u_int arg4, arg5;
	/* Exercise 4.2: Your code here. (3/4) */
	arg4 = *((u_int *) (tf->regs[29] + 16));
	arg5 = *((u_int *) (tf->regs[29] + 20));
	/* Step 5: 调用处理函数，返回值保存在用户态的 v0 中
	 */
	/* Exercise 4.2: Your code here. (4/4) */
		tf->regs[2] = func(arg1, arg2, arg3, arg4, arg5);
}
```

### 用户栈与内核栈

- 这里 sysno 取自 a0 寄存器，那前面说的 tf 地址保存在 a0 寄存器又是什么呢，不会互相覆盖吗？
- 首先结论很明显，不会。这里的两个 a0 指的不是同样的东西

首先我们需要明确一点：当进程运行在用户态时，使用的是用户栈，栈指针也指向用户栈；每当进程**通过异常**从用户态切换到内核态时，handler 会执行汇编函数 `SAVE_ALL`。

它的具体作用是把用户态的所有寄存器都保存到一个 trapframe 中，同时这个 tf 会被放置在内核栈中，同时还会切换当前使用的栈空间为**内核栈**。并且**切换时内核栈总是空的**。内核栈此时就保存了进程在进入内核态前的相关信息。重回到用户态时，再通过 ret_from_exception 中的 RESTORE_SOME 将内核栈中保存的信息恢复，再切回用户栈。

因为内核栈在切换后总是空的，每次又只会传入一个 trapframe ，所以这个 trapframe 实际上每次都占用的是 KSTACKTOP 向下的一个 sizeof(trapframe) 大小的空间

所以会出现这样的空间图：

- stackframe：调用函数时创建，保存函数的参数、临时变量与相关跳转指针
- trapframe：陷入内核时使用 `SAVE_ALL` 创建，保存用户态寄存器

stackframe（用户栈）：para3 para2 para1 （自 msyscall 传入）

trapframe：用户态的 \$1 \$2 … \$sp \$30 \$31

sp（内核栈）：栈顶 trapframe（自SAVE_ALL压入）

新的内核栈 sp 指向 trapframe，旧的用户栈 sp 指向 para1

- 使用当前（内核栈）的 sp，能访问到 trapframe 的信息；使用 tf 中 sp （用户栈）的信息，能访问到最近的 stackframe 的信息

回到我们的 sysno，它是 `msyscall` 的第一个参数（**用户态**函数），也就是在分发异常调用 SAVE_ALL 时保存的 a0 寄存器，来源是用户态，所以要从用户态的 tf 里取 a0 寄存器（tf->regs[4]）；而参数 \*tf 是保存在了调用 `do_syscall` 时的内核态 a0 中，并不干扰

- 后面的 arg4 类似，同样来源于用户态，但由于寄存器内没有保存，所以不能直接从 tf 里取得，需要通过用户栈指针回到 stackframe 中获取(tf->regs[29] + 16)

### 栈帧 - stack frame

这里是实际在网站里的教程里有讲过了，感觉还是再说说吧。

昨天有同学问我说为啥NESTED(handler_sys, TFSIZE + 8, 0)提示编译器共有 TFSIZE + 8 字节的栈帧，但是 ra 却只移动了 8Byte。然后我就发现，我确实没理解栈帧。。

存疑：栈帧在C语言函数中自动创建，汇编函数中需要手动创建

以下内容根据 R3000 手册筛选，但不能保证正确性，为了讨论，这里就只涉及到非叶函数，也就是一般函数的栈情况

- 栈帧 stackframe 创建于刚刚进入函数时：编译器会令 sp 指针向下移动一定空间，并使得这段空间成为该函数栈帧（大小由编译器通过函数变量、子函数参数、临时变量等指标确定；需要**注意双字对齐**，有空白时需要补充空白字（称作pad））
- 随即，编译器调用 sw 指令，把 a0 - a3 寄存器中存放的本函数前四个参数填充到紧邻本栈帧的**上方的空间**里，不够四个就有几个补几个
- 栈内高地址存放函数的临时变量等，低地址预留本函数可能调用的**子函数的参数**的空间，这部分空间在进入子函数时又会成为子函数第二步填充参数的地方

那回来说这个 handler_sys，它通过NESTED宏中的.frame向编译器声明自己需要 TFSIZE+8 字节的栈帧，但是自己却只移动了 8 字节的 sp。

我感觉是因为 TFSIZE 实际上是上面调用 SAVE_ALL 时移动的一个 TFSIZE，这里为了避免覆盖掉就把它看成了栈的一部分（小孩子瞎猜的）。真正有用的是那8个字节：4字节的arg1和4字节的pad，而arg1又是在进入子函数do_syscall内才填充的

## 系统调用函数

了解完系统调用的流程后，下一步就是填写具体用于处理系统调用的函数了，可能会把能写的都写一下，建议是跳着看。



### 获取进程块 - envid2env -Exercise 4.3

首先是 envid2env 这个函数，它用来获取id对应的进程控制块。虽然它不是系统调用的一部分，但在进行交互、系统调用时，经常使用

```c
int envid2env(u_int envid, struct Env **penv, int checkperm) {
  struct Env *e;

  /* Step 1: 赋值，当 envid == 0 时返回当前进程块 */
  /* Exercise 4.3: Your code here. (1/2) */
  if (envid == 0) {
    *penv = curenv;
    return 0;
  }
  e = &envs[ENVX(envid)];

  if (e->env_status == ENV_FREE || e->env_id != envid) {  // double check: invaild env_id
    *penv = 0;
    return -E_BAD_ENV;
  }

  /* Step 2: Check when 'checkperm' is non-zero. */
  /* 
   * 当 checkperm != 1 时，要求查询的 env 必须是当前运行进程块**本身或父亲**
   */
  /* Exercise 4.3: Your code here. (2/2) */
  if (checkperm && (e->env_id != curenv->env_id && e->env_parent_id != curenv->env_id)) {
    *penv = 0;
    printk("E_BAD_ENV: %x, %x, %x\n", e->env_id, curenv->env_id, e->env_parent_id);
    return -E_BAD_ENV;
  }
  /* Step 3: 向 *penv 中赋值 */
  *penv = e;
  return 0;
}
```

函数本身没什么问题，但要注意 `envid == 0` 时必须提前退出函数，否则会一直运行到最后，返回 envs 内的第一个进程控制块

- 当 checkperm == 0 时，不需处理进程块与当前进程之间的关系，反之则需要确保调用本函数的进程是被调用者的**直接父亲或本身**，否则返回错误。值得一提的是，除了在进行 ipc 通讯的过程外，所有 syscall 函数都需要令 checkperm != 0（传信息不用限定在父子进程中）



### 打印字符至控制台 - sys_putchar

直接调用了 `printcharc` 函数，和 `printk` 效果类似吧

```c
void sys_putchar(int c) {
	printcharc((char)c);
	return;
}
```



### 打印用户空间的定长字符串 - sys_print_cons

先检查了地址是否位于用户区域，然后检查了长度是不是正的，最后循环调用了 `printcharc` 函数

```c
int sys_print_cons(const void *s, u_int num) {
	if (((u_int)s + num) > UTOP || ((u_int)s) >= UTOP || (s > s + num)) {
		return -E_INVAL;
	}
	u_int i;
	for (i = 0; i < num; i++) {
		printcharc(((char *)s)[i]);
	}
	return 0;
}
```



### 获得运行中进程块的 id - sys_getenvid

就一句话，但是好像还没有在 MOS 中使用过

```c
u_int sys_getenvid(void) {
	return curenv->env_id;
}
```



### 强制进行进程切换 - sys_yield - Exercise 4.7

具体而言就是调用一次 `schedule` 函数，使得运行的进程交出 CPU 时间片，记得 yield 别传 0（）

```c
void __attribute__((noreturn)) sys_yield(void) {
	// Hint: Just use 'schedule' with 'yield' set.
	/* Exercise 4.7: Your code here. */
	schedule(1);
}
```

需要注意这里函数 `noreturn` ，也就是说会直接开始运行下一个进程块



### 销毁进程 - sys_env_destroy

只有直系进程才能进行销毁（自己也行）

```c
int sys_env_destroy(u_int envid) {
	struct Env *e;
	try(envid2env(envid, &e, 1));

	printk("[%08x] destroying %08x\n", curenv->env_id, e->env_id);
	env_destroy(e);
	return 0;
}
```



### 设置当前进程的页写入异常处理函数 - sys_set_tlb_mod_entry - Exercise 4.12

允许用户指定页写入异常处理函数（函数放在用户态中）

```c
int sys_set_tlb_mod_entry(u_int envid, u_int func) {
	struct Env *env;

	/* Step 1: 获取目标进程块 */
	/* Exercise 4.12: Your code here. (1/2) */
	try(envid2env(envid, &env, 1));
	/* Step 2: 设置 'env_user_tlb_mod_entry' 地址为 'func'. */
	/* Exercise 4.12: Your code here. (2/2) */
	env->env_user_tlb_mod_entry = func;
	return 0;
}
```



### 指定进程建立 va2pa 的映射 - sys_mem_alloc - Exercise 4.4

- 函数作用：为指定进程的 va 申请一个物理页面并形成映射
  - 类似于跨进程的 page_insert
- 操作：
  - 确认 va 和 env 的合法性，如有错误则直接返回错误值
  - 申请一个物理页，调用 page_insert 生成映射

```c
int sys_mem_alloc(u_int envid, u_int va, u_int perm) {
	struct Env *env;
	struct Page *pp;

	/* Step 1: 检查 va 合法性 */
	/* Exercise 4.4: Your code here. (1/3) */
	if (is_illegal_va(va)) {
		return -E_INVAL;
	}
	/* Step 2: 获取 envid 的进程块 */
	/* Hint: **Always** validate the permission in syscalls! except for in function sys_ipc_try_send */
	/* Exercise 4.4: Your code here. (2/3) */
	if (0 != envid2env(envid, &env, 1)) {	// test 4-2: !envid2env() or 0 != envid2env())
		return -E_BAD_ENV;
	}
	/* Step 3: 申请物理页 */
	/* Exercise 4.4: Your code here. (3/3) */
	try(page_alloc(&pp));
	/* Step 4: 将 va 映射入指定进程 */
	return page_insert(env->env_pgdir, env->env_asid, pp, va, perm);
}
```

乍一看这不就是一个 page_insert 吗，仔细一看确实。但是用户态的 page_insert 无法**帮**其他进程申请一个映射，因为根本看不到其他进程的进程块，所以原则上需要看得见所有东西的内核态来帮忙



### 在不同进程间形成共同的映射 - sys_mem_map - Exercise 4.5

- 函数作用：说白了就是把 src 进程 va 所在的物理页，在 dst 进程中找了指定位置形成了映射（insert）
- 操作：
  - 检验传入的两个 va 合法性
  - 获取 srcid 和 dstid 的进程控制块
  - page_lookup 获得 srcva 在 srcid 中映射的**物理页**
  - page_insert 让物理页在 dstid 中也形成一个映射

```c
int sys_mem_map(u_int srcid, u_int srcva, u_int dstid, u_int dstva, u_int perm) {
	struct Env *srcenv;
	struct Env *dstenv;
	struct Page *pp;
	//printk("mem_map:va is %x\n", srcva);
	/* Step 1: 检查 va 合法性 */
	/* Exercise 4.5: Your code here. (1/4) */
	if (is_illegal_va(srcva) || is_illegal_va(dstva)) {
		// printk("invaild va: %x or %x\n", srcva, dstva);
		return -E_INVAL;
	}
	/* Step 2: 获取 srcid 的进程块 */
	/* Exercise 4.5: Your code here. (2/4) */
	if (envid2env(srcid, &srcenv, 1)) {
		// printk("bad env2! envid2env(%x, &srcenv, 1)\n", srcid);
		return -E_BAD_ENV;
	}
	/* Step 3: 获取 dstid 的进程块 */
	/* Exercise 4.5: Your code here. (3/4) */
	if (envid2env(dstid, &dstenv, 1)) {
		// printk("bad env2! envid2env(%x, &dstenv, 1)\n", dstid);
		return -E_BAD_ENV;
	}
	/* Step 4: 找到 srcid + srcva 指向的物理页 */
	/* Return -E_INVAL if 'srcva' is not mapped. */
	/* Exercise 4.5: Your code here. (4/4) */
	if ((pp = page_lookup(srcenv->env_pgdir, srcva, NULL)) == NULL) {
		return -E_INVAL;
	}
	/* Step 5: 最终在 dst 中形成映射 */
	return page_insert(dstenv->env_pgdir, dstenv->env_asid, pp, dstva, perm);
}
```



### 解除指定进程的映射 - sys_mem_unmap - Exercise 4.6

- 函数作用：上一行就是.jpg
- 操作：
  - 检验 va 有效性
  - 获取 envid 的进程块
  - 直接调用 page_remove 进行映射的删除

```c
int sys_mem_unmap(u_int envid, u_int va) {
	struct Env *e;

	/* Step 1: 检查 va 合法性 */
	/* Exercise 4.6: Your code here. (1/2) */
	if (is_illegal_va(va)) {
		return -E_INVAL;
	}
	/* Step 2: 获取对应的进程控制块 */
	/* Exercise 4.6: Your code here. (2/2) */
	if (envid2env(envid, &e, 1)) {
		return -E_BAD_ENV;
	}
	/* Step 3: 解除 env 中 va 的映射 */
	page_remove(e->env_pgdir, e->env_asid, va);
	return 0;
```

比较好写，但是 MOS 也没调用过，之后可能会有用处吧



### 为当前进程创建一个子进程 - sys_exofork - Exercise 4.9



```c
int sys_exofork(void) {
	struct Env *e;

	/* Step 1: 使用 env_alloc 申请新进程块 */
	/* Exercise 4.9: Your code here. (1/4) */
	try(env_alloc(&e, curenv->env_id));
	/* Step 2: 把当前进程存入的 trapframe 复制给子进程 */
	/* Exercise 4.9: Your code here. (2/4) */
	e->env_tf = *((struct Trapframe *)KSTACKTOP - 1);
	// e->env_tf = curenv->env_tf;
	/* Step 3: 把子进程的 v0 寄存器置0，即函数返回值为0 */
	/* Exercise 4.9: Your code here. (3/4) */
	e->env_tf.regs[2] = 0;
	/* Step 4: 设置子进程状态，继承优先级  */
	/* Exercise 4.9: Your code here. (4/4) */
	e->env_status = ENV_NOT_RUNNABLE;
	e->env_pri = curenv->env_pri;
	return e->env_id;
}
```



### 设置进程块 status - sys_set_env_status



```c
int sys_set_env_status(u_int envid, u_int status) {
	struct Env *env;

	/* Step 1: 检查 va 合法性 */
	/* Exercise 4.14: Your code here. (1/3) */
	if (status != ENV_RUNNABLE && status != ENV_NOT_RUNNABLE) {
		return -E_INVAL;
	}
	/* Step 2: 获取 envid 得进程控制块 */
	/* Exercise 4.14: Your code here. (2/3) */
	try(envid2env(envid, &env, 1));
	/* Step 4: 如果 RUNNABLE 就插入调度链表 */
	/* Exercise 4.14: Your code here. (3/3) */
	if (status == ENV_RUNNABLE) {
		TAILQ_INSERT_TAIL(&env_sched_list, env, env_sched_link);
	}
	/* Step 5: 设置 'env_status' */
	env->env_status = status;
	return 0;
}
```



### 设置 trapframe - sys_set_trapframe

**页写入异常时要用，先不研究呢**

```c
int sys_set_trapframe(u_int envid, struct Trapframe *tf) {
	if (is_illegal_va_range((u_long)tf, sizeof *tf)) {
		return -E_INVAL;
	}
	struct Env *env;
	try(envid2env(envid, &env, 1));
	if (env == curenv) {
		*((struct Trapframe *)KSTACKTOP - 1) = *tf;
		// return `tf->regs[2]` instead of 0, because return value overrides regs[2] on
		// current trapframe.
		return tf->regs[2];
	} else {
		env->env_tf = *tf;
		return 0;
	}
}
```



### ipc 通信：接收端进程 - sys_ipc_recv - Exercise 4.8



```c
int sys_ipc_recv(u_int dstva) {
	/* Step 1: 非 0 va 意味着传输页面，则需检测是否合法 */
	if (dstva != 0 && is_illegal_va(dstva)) {	// test 4-3: English "either or"
		return -E_INVAL;
	}

	/* Step 2: 标记：允许接收数据 */
	/* Exercise 4.8: Your code here. (1/8) */
	curenv->env_ipc_recving = 1;
	/* Step 3: 设置接收的指定va */
	/* Exercise 4.8: Your code here. (2/8) */
	curenv->env_ipc_dstva = dstva;
	/* Step 4: 移出调度队列，进入阻塞态 */
	/* Exercise 4.8: Your code here. (3/8) */
	curenv->env_status = ENV_NOT_RUNNABLE;
	TAILQ_REMOVE(&env_sched_list, curenv, env_sched_link);
	/* Step 5: schedule(1) 开摆，设置返回值为0 */
	((struct Trapframe *)KSTACKTOP - 1)->regs[2] = 0;
	schedule(1);
}
```



### ipc 通信：发送端进程 - sys_ipc_send - Exercise 4.8







### 读入字符 - sys_cgetc

直接调用了函数 `scancharc` ，此时会让系统处于忙等状态，直至接收到字符返回

```c
int sys_cgetc(void) {
	int ch;
	while ((ch = scancharc()) == 0) {
	}
	return ch;
}
```

