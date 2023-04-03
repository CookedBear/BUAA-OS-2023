## 思考题



### Thinking 2.1

> 虚拟地址在系统中的使用

根据指导书说明：

> 而在实际程序中，访存、跳转等**指令**以及用于取指的**PC寄存器**中的访存地址都是**虚拟地址**。
>
> 我们编写的C程序中也经常通过对指针解引用来进行访存，其中**指针的值**也会被视为**虚拟地址**，经过编译后生成相应的访存指令。

故在编写的C程序中指针存储的地址和汇编程序中`lw`和`sw`使用的值都是**虚拟地址**。

![image-20230326171214176](https://cookedbear-2003-1307884465.cos.ap-beijing.myqcloud.com/NotePics/202303261712115.png)

### Thinking 2.2

> 链表宏与对应的链表结构

- 从可重用性的角度，阐述用宏来实现链表的好处。

使用宏进行链表实现，一方面能够规范系统内所有链表的存在形式，达成结构上的统一，易于编码与链表维护；

链表宏中大部分传入的参数并不是链表项整体，而是不包含链表具体存放信息的一个**结构体**，这个结构体只负责管理链表指针。

我们利用宏对这个结构体进行处理，一方面统一链表操作，不限制可以使用链表的结构类型；另一方面能够降低链表指针与链表内容间不必要的耦合性，维护数据安全；使用预设结构还能保证链表结构的正确性。

查阅资料后得知，除了重用性角度外，使用宏定义还意味着放置在`queue.h`内的所有宏都是完整的，不需要编译为**库文件**

- 分析**单向链表**、**循环链表**和实验中使用的**双向链表**在插入与删除操作上的性能差异。

> 头文件`queue.h`为C语言中的链表提供了更加标准规范的编程接口。如今的版本多为伯克利加州大学1994年8月的8.5版本。
>
> 每种结构都支持基本的操作：
>
> 1. 在链表头插入节点
> 1. 在任意的节点后插入节点
> 1. 移除链表头后的节点
> 1. 前向迭代遍历链表

我们打开对应的文件`/usr/include/sys/queue.h`，可以发现其中不仅仅包含了我们使用的双向链表`LIST`、待分析的单向链表`SLIST`和循环链表`CIRCLEQ`，还有两个有尾链表结构`TAILQ`和`SIMPLEQ`（双向有尾/单向有尾），这5个数据结构共同构成了`LINUX`的链表宏定义（为节省空间，下文只拿出所需的定义进行解释，其他定义便不再详述**，完整定义会贴在全文最后**）

1. 单向链表`SLIST`：

单向链表（`Singly-linked List`）在结构上较为简单，链表项只包含一个指向下一个元素的指针

![单向链表](https://cookedbear-2003-1307884465.cos.ap-beijing.myqcloud.com/NotePics/202303260156076.png)



```c
#define	SLIST_ENTRY(type)										\
struct {														\
	struct type *sle_next;	/* next element */					\
}
```

- 插入操作：
  - 链表中插入（已知前驱元素指针）需要修改被插入项`slistelm`和自身`elm`的指针，即需要两条指令；
  - 插入第一个元素需要修改自身`elm`的指针指向原有的第一项，同时还需要修改**链表头项**的`slh_first`指向自身，也需要两条指令
- 删除操作：
  - 链表中删除（无前驱元素指针）需要判断是否前驱元素为链表第一个元素，是则归入第二类，不是则需要从头部开始遍历得到前驱元素，并修改前驱指针指向自身的后继元素，需要$2n+2$条指令
  - 删除第一个元素只需要修改链表头指向的链表项即可，即一条指令

```c
#define	SLIST_INSERT_AFTER(slistelm, elm, field) do {			\
// 执行2条语句
    
#define	SLIST_INSERT_HEAD(head, elm, field) do {				\
// 执行2条语句
    
#define	SLIST_REMOVE_HEAD(head, field) do {						\
// 执行1条语句
    
#define	SLIST_REMOVE(head, elm, type, field) do {				\
// 执行1或2n+2条语句
```

2. 循环链表`CIRCLEQ`：

`LINUX`宏定义下的循环链表只存在双向循环链表`CIRCLEQ`一种，它是`Circular queue`的简写

![循环链表](https://cookedbear-2003-1307884465.cos.ap-beijing.myqcloud.com/NotePics/202303260200417.png)

由于循环链表有头有尾，故链表头项包含两个指针：`cqe_first`和`cqe_last`

```c
#define CIRCLEQ_HEAD(name, type)                                            \
struct name {                                                               \
    struct type *cqh_first;     /* first element */                         \
    struct type *cqh_last;      /* last element */                          \
}
```

同时每个链表项与双向无尾链表`LIST`结构稍有不同，它有两个指向链表项的指针:`cqe_next`和`cqe_prev`

```c
#define CIRCLEQ_ENTRY(type)                                                 \
struct {                                                                    \
    struct type *cqe_next;      /* next element */                          \
    struct type *cqe_prev;      /* previous element */                      \
}
```

由于可以提供双向+头尾指针，所以循环链表可以有多种插入方式：

```c
#define CIRCLEQ_INSERT_AFTER(head, listelm, elm, field) do {                \
// 执行4条语句
#define CIRCLEQ_INSERT_BEFORE(head, listelm, elm, field) do {               \
// 执行4条语句
#define CIRCLEQ_INSERT_HEAD(head, elm, field) do {                          \
// 执行4条语句
#define CIRCLEQ_INSERT_TAIL(head, elm, field) do {                          \
// 执行4条语句
#define CIRCLEQ_REMOVE(head, elm, field) do {                               \
// 执行2条语句，但需要进行额外的两次if分支判断
```

在进行插入操作时，需要考虑到是否仅含一个元素的情况，这时需要对链表头项进行修改，与其他情况稍有不同（以INSERT_HEAD为例）

```c
if ((head)->cqh_last == (void *)(head))                              		\
        (head)->cqh_last = (elm);                                           \
    else                                                                    \
        (head)->cqh_first->field.cqe_prev = (elm);                          \
    (head)->cqh_first = (elm);  
```

3. 双向链表`LIST`：

然后就是我们在实验中使用到的双向链表`LIST`，它与其他内置链表宏的最大区别就是链表项：`LIST`含有一个指针和一个**二重指针**

```c
#define LIST_ENTRY(type)                                                    \
struct {                                                                    \
    struct type *le_next;   /* next element */                              \
    struct type **le_prev;  /* address of previous next element */          \
}
```

也因为其能够进行双向操作，插入的方法也有多种

```c
#define LIST_INSERT_AFTER(listelm, elm, field) do {                         \
// 执行4条语句

#define LIST_INSERT_BEFORE(listelm, elm, field) do {                        \
// 执行4条语句

#define LIST_INSERT_HEAD(head, elm, field) do {                             \
// 执行4条语句

#define LIST_REMOVE(elm, field) do {                                        \
// 执行2条语句
```

这里需要注意执行向后插入的`INSERT_AFTER`和`INSERT_HEAD`都需要判断是否为最后一个元素，决定是否需要调整后继元素的`le_prev`指针

分析后可以看出，

- 单项链表`SLIST`执行插入的效率最高，但是删除时需要进行遍历操作以防止后续链表元素丢失，性能较差；
- 循环链表`CIRCLEQ`执行插入的效率稍低于单项链表，删除效率要高于单向链表，但需要进行分支判断，也为常数级
- 双向链表`LIST`插入效率与循环链表类似，删除效率略高于循环链表

### Thinking 2.3

查看`pmap.h`得知`Page_list`结构调用了`LIST_HEAD()`宏定义

```c
LIST_HEAD(Page_list, Page);

// 具体替换后结构如下
#define LIST_HEAD(Page_list, Page)                                               	\
	struct Page_list {                                                             	\
		struct Page *lh_first; /* first element */                          	\
	}
```

同时还定义了`Page`结构体和`Page_LIST_entry_t`：

```c
struct Page {
	Page_LIST_entry_t pp_link; /* free list link */
	u_short pp_ref;
};
typedef LIST_ENTRY(Page) Page_LIST_entry_t;

// 替换后结构如下
#define LIST_ENTRY(Page)                                                                   \
	struct {                                                                               \
		struct Page *le_next;  /* next element */                                          \
		struct Page **le_prev; /* address of previous next element */                      \
	}
```

于是可以得出完整结构：

```c
struct Page_list {
    struct {			// Page
        struct {		// Page_LIST_entry_t
            struct Page *le_next;
            struct Page **le_prev;
        } pp_link;
        
        u_short pp_ref;
    }* lh_first
}
```

故选择C选项



### Thinking 2.4

- 请阅读上面有关R3000-TLB 的描述，从虚拟内存的实现角度，阐述ASID 的必要性。

> ASID，Address Space IDentifier 用于区分不同的地址空间，同一虚拟地址通常在不同的地址空间中映射到不同的物理地址。

也就是说，ASID用来标记当前虚拟地址所归属的程序号，如果访问的是其他程序的虚拟地址，通常会指向一个错误的物理地址。因此需要确保访问的请求来自特定的程序，于是采用了ASID作为保险，确定虚拟地址请求的来源。

- 请阅读《IDT R30xx Family Software Reference Manual》的Chapter 6，结合ASID段的位数，说明R3000 中可容纳不同的地址空间的最大数量。

ASID段占用的数据位为[11, 6]，共6位，则意味着R3000中可容纳不同的地址空间的最大数量为64。

### Thinking 2.5

- tlb_invalidate 和tlb_out 的调用关系？

打开`kern/pmap.c`文件并找到`tlb_invalidate`函数定义如下：

```c
void tlb_invalidate(u_int asid, u_long va) {
    tlb_out(PTR_ADDR(va) | (asid << 6));
}
```

易知C语言函数`tlb_invalidate`调用了汇编函数`tlb_out`

- 请用一句话概括tlb_invalidate 的作用。

删除在序号为`ASID`的程序规划的虚拟空间中，虚拟地址`va`所在页在`TLB`中的页表项

- 逐行解释tlb_out 中的汇编代码。

`tlb_out`代码如下

```assembly
LEAF(tlb_out)
.set noreorder										# 禁止代码优化
	mfc0 t0, CP0_ENTRYHI				# 从CP0_ENTRYHI寄存器取值至$t0
	mtc0 a0, CP0_ENTRYHI				# 将$a0的值存入CP0_ENTRYHI中
	nop
	tlbp								# 查找 TLB 表项，将索引存入 Index 寄存器
	nop
	mfc0 t1, CP0_INDEX					# 将索引取值至$t1寄存器
.set reorder										# 优化代码
	bltz t1, NO_SUCH_ENTRY				# 索引空，准备退出
.set noreorder										# 禁止代码优化
	mtc0 zero, CP0_ENTRYHI				# 清空ENTRYHI和ENTRYLO
	mtc0 zero, CP0_ENTRYLO0
	nop
	tlbwi								# 清空 Index 对应的 TLB 项
.set reorder										# 优化代码

NO_SUCH_ENTRY:
	mtc0 t0, CP0_ENTRYHI				# 把ENTRYHI的值回存
	j ra
END(tlb_out)										# 结束 tlb_out 函数
```



### Thinking 2.6

- 简单了解并叙述X86 体系结构中的内存管理机制，比较X86 和MIPS 在内存管理上的区别。

在x86架构中内存被分为三种形式，分别是**逻辑地址**（Logical Address），**线性地址**（Linear Address）和**物理地址**（Physical Address）。其中分段可以将逻辑地址转换为线性地址，而分页可以将线性地址转换为物理地址。

并且x86允许存在不分页的情况，但不允许程序不分段

**MIPS 与 X86 的 TLB 差别**

其在于对 TLB 不命中时的处理上：

MIPS 会触发TLB Refill 异常，内核的 tlb_refill_handler 会以 pgd_current 为当前进程的 PGD 基址，索引获得转换失败的虚址对应的 PTE，并将其填入 TLB，完了CPU 把刚刚转换失败的虚地址再走一下 TLB 就OK了。

而 X86 在 TLB 不命中时，是由硬件 MMU 以 CR3 为当前进程的 PGD 基址，索引获得 PFN 后，直接输出 PA。同时 MMU 会填充 TLB 以加快下次转换的速度。



























## 实验细节

在进行实验前，建议通读`mmu.h`和`pmap.h`文件，了解一些宏定义，对后续工作很有帮助

|     名称      |             参数              |                          作用                          |
| :-----------: | :---------------------------: | :----------------------------------------------------: |
|     PADDR     |      内核**虚拟地址**kva      |          将内核虚拟地址kva转成对应的物理地址           |
|     KADDR     |        **物理地址**pa         |             将物理地址pa转化为内核虚拟地址             |
|    page2pa    |   页信息结构**struct Page**   |       通过空闲页结构得到这一页起始位置的物理地址       |
|    pa2page    |        **物理地址**pa         |   通过物理地址pa获取这一页对应的页结构体struct Page    |
|   page2kva    | 页信息结构**struct PageInfo** |       通过空闲页结构得到这一页起始位置的虚拟地址       |
|      PDX      |        **线性地址**la         |           获得该线性地址la对应的页目录项索引           |
|      PTX      |        **线性地址**la         |      获得该线性地址la在二级页表中对应的页表项索引      |
| PTE_ADDR(pte) |   页表项或页目录项的**值**    | 获得对应的页表/地址基址(低12位为0，并均为**物理地址**) |

### 内核启动后的内存分配 - 2.1

- 我们的操作系统内核启动后需要执行下列三个函数完成内存分配机制的建立
  - `mips_detect_memory()`：探测硬件可用内存，初始化内存基本变量
  - `mips_vm_init()`：建立用于管理内存的数据结构
  - `page_init()`：初始化结构`Page`与空闲链表`page_free_list`

#### `mips_detect_memory()` - Exercise 2.1

- 本函数的作用是探测硬件可用内存，并初始化两个变量：
  - `memsize`：物理内存对应的字节数（实验中从外设中读取）
  - `npage`：物理内存对应的页数

- 请参考代码注释，补全`mips_detect_memory`函数。
- 在实验中，从外设中获取了硬件可用内存大小`memsize`，请你用内存大小`memsize`完成总物理页数`npage`的初始化

相对简单的一题，`memsize`已知，只需要除以每页的大小即可（`BY2PG = 4096`定义在`mmu.h`中）

在这里要留意一下`npage`这个变量，后面的练习中会用到几次

```c
void mips_detect_memory() {
    memsize = *(volatile u_int *)(KSEG1 | DEV_MP_ADDRESS | DEV_MP_MEMORY);
    /* Exercise 2.1 Your code here. */
    npage = memsize / BY2PG;
    
    printk("Memory size: %lu KiB, number of pages: %lu\n", memsize / 1024, npage);
}
```

#### `mips_vm_init()`

- 本函数通过一次性调用`alloc()`函数完成对内存管理结构（**页控制块**）所在空间的初步分配，实现的关键在`alloc()`中。这个函数只在系统初始化、尚未生成页式管理机制时才会被调用。

```c
// 在建立页式存储机制前，使用alloc函数进行内存空间的分配
static void *alloc(u_int n, u_int align, int clear) {
    extern char end[];
    /* Initialize 'freemem' if this is the first time. */
    if (freemem == 0) {
        freemem = (u_long)end;			/* 明确 freemem 指向虚拟地址 */
    }
    freemem = ROUND(freemem, align);	/* 按大小向上取整，保证此时freemem表示
    									 * 空闲空间的起始地址
    									 */
    alloced_mem = freemem;				/* freemem 以下空间均被分配 */
    freemem = freemem + n;				/* 分配出 n 个字节大小的空间 */
    if (PADDR(freemem) >= memsize) {	/* 物理地址越界，无足够空闲空间 */
        panic("out of memory");
    }							
    if (clear) {						/* clear 是分配内存清零的标志位 */
        memset((void *)alloced_mem, 0, n);
    }
    return (void *) alloced_mem;		/* 返回被分配空间的起始**虚拟**地址 */
}

void mips_vm_init() {
    pages = (struct Page *)alloc(npage * sizeof(struct Page), BY2PG, 1);
    /* 分配了一块大小为 (npage * 页表项) 字节的空间并清空，用于存放所有的页表项
     * 起始地址为 pages
     */
}
```

- `freemem`：在这里我们通过使用虚拟内存的`kseg0`段来操作内存，但实际上它可以直接映射到指定的物理地址（去掉最高位），不需要额外的映射逻辑
- `extern char end[]`：在lab1中分配段加载地址用到的`kernel.lds`已经声明过，可以直接使用表示地址

```
. = 0x80400000;	// 相当于物理内存的0x400000
end = . ;
```



### 物理内存管理 - 2.2 ~ 2.5

- MOS系统使用页式内存管理物理内存，使用链表管理页表，重要代码位于`kern/pmap.c`中

#### 链表宏 - Exercise 2.2

更具体的链表宏其实在Thinking部分已经提到了，这里不再过多展开。

在这里出现的链表形式与常见的双向链表有些不同，每一个链表项含有一个`field* le_next`，还有一个`field** le_prev`，其中的`le_next`很好理解，就正常指向下一个链表项，但是这个`le_prev`则是指向**上一个**链表项的`le_next`域：更直观地来说就是这样

```c
first->field.le_next = second
second->field.le_prev = &(first->field.le_next)
// **类型 = 给*类型取地址
*(second->field.le_prev) = first->field.le_next = second
```

至此，Exercise 2.2已经可以完成了

```c
#define LIST_INSERT_AFTER(listelm, elm, field) do {                         \
    if (((elm)->field.le_next = (listelm)->field.le_next) != NULL)          \
        (listelm)->field.le_next->field.le_prev =                           \
            &((elm)->field.le_next);                                        \
    (listelm)->field.le_next = (elm);                                       \
    (elm)->field.le_prev = &(listelm)->field.le_next;                       \
} while (0)	
```

![image-20230331180129442](https://cookedbear-2003-1307884465.cos.ap-beijing.myqcloud.com/NotePics/202303311801671.png)

再有一个注意点就是，各个宏定义传入参数时要求的是什么类型：

`head`参数就要求传入的是链表头的指针`&`（本实验中指`Page_List *`）

`elm`参数要传入链表项的指针`&`（本实验中指`Page *`）

`field`参数则传入的是链表项的指针（本实验中指的是`pp_link`）



> 接下来的三个练习（2.3-2.5）针对的是在建立页式内存管理后中/后所需要用到的一些函数，这些函数的正确与否直接决定了分页管理能否正常运行（虽然`init`那里错了也会寄就是了，哈哈比如我）

#### `page_init()` - Exercise 2.3

- `page_init()`函数用于初始化空闲链表`page_free_list`

任务要求为完成函数：

1. 使用链表初始化宏`LIST_INIT`。
1. 将`freemem`按照`BY2PG`进行对齐（使用`ROUND`宏为`freemem`赋值）。
1. 将`freemem`以下页面对应的页控制块中的`pp_ref`标为1。
1. 将其它页面对应的页控制块中的`pp_ref`标为0 并使用`LIST_INSERT_HEAD`将其插入空闲链表。

还可以这样把任务具体化：

1. 对`free_page_list`初始化，使用`LIST_INIT()`宏
1. 将`freemem`按`BY2PG`对齐，使用`ROUND()`宏
1. 从划分的页表项指针`pages`开始，按照地址由低到高遍历：
   1. `virtual address < freemem`：令`pp_ref = 1`
   1. `else`：令`pp_ref = 0`、使用`LIST_INSERT_HEAD()`宏插入`free_page_list`中

将任务具体化后，发现实际上前两步的实现相对简单，使用了头文件定义好的宏，或是复现之前出现过的操作，就不多展开了。重点在最后一步的遍历`Page *pages`上：

首先，循环可以使用`pages++`方法（`+`运算符重载），同样也可以使用`pages[i], i++`的做法，这样都能表示单次大小为`page`的地址位移

其次，直观地想要实现`virtual address < freemem`的比较，需要获取到每一页的虚拟地址，实际**上`pmap.h`提供了`page2kva`这个返回虚拟地址的宏**



当然在这里也可以换个思路，不去转换`PPN`和`pages`，而把`freemem`这个虚拟地址转化为页表项相关的值，也就是找到`freemem`对应的`PPN`号：`PPN(freemem)`，然后进行循环边界的判断即可

具体实现如下：

```c
void page_init(void) {
  /* Step 1: Initialize page_free_list. */
  /* Hint: Use macro `LIST_INIT` defined in include/queue.h. */
  /* Exercise 2.3: Your code here. (1/4) */
  LIST_INIT(&page_free_list);
  /* Step 2: Align `freemem` up to multiple of BY2PG. */
  /* Exercise 2.3: Your code here. (2/4) */
  freemem = ROUND(freemem, BY2PG);
  /* Step 3: Mark all memory below `freemem` as used (set `pp_ref` to 1) */
  /* Exercise 2.3: Your code here. (3/4) */
  struct Page *p = pages;
  int i;
  int freePPN = PPN(PADDR(freemem));
    
  for (i = 0; i < freePPN; i++) {
    pages[i].pp_ref = 1;
  }
  /* Step 4: Mark the other memory as free. */
  /* Exercise 2.3: Your code here. (4/4) */
  for (; i < npage; i++) {
    pages[i].pp_ref = 0;
    LIST_INSERT_HEAD(&page_free_list, (pages + i), pp_link);
  }
}
```



#### `page_alloc` - Exercise 2.4

- 如果能顺利完成并理解前面练习的话，这个也理应算是个水题了（

任务要求为完成`page_alloc()`函数，这个函数取代了最初的`alloc()`函数，成为页式内存管理中申请内存时调用的函数

1. 如果空闲链表没有可用页了，返回异常返回值。
1. 如果空闲链表有可用的页，取出第一页；初始化后，将该页对应的**页控制块的地址**放到调用者指定的地方。
1. 可能需要使用链表宏`LIST_EMPTY`或函数`page2kva`
  1. 实际上Use `LIST_FIRST` and `LIST_REMOVE`也在函数前面的注释里当作HINT了（
  1. 顺带提一嘴，异常返回值也在那个注释写了，叫`-E_NO_MEM`

这个也就做一下空链表判断返回值，再取出**第一页**并清空即可，**不要忘记清空的区域大小是多少**

```c
int page_alloc(struct Page **new) {
  /* Step 1: Get a page from free memory. If fails, return the error code.*/
  struct Page *pp;
  /* Exercise 2.4: Your code here. (1/2) */
  if (LIST_EMPTY(&page_free_list)) {
    return -E_NO_MEM;
  }
  pp = LIST_FIRST(&page_free_list);
  LIST_REMOVE(pp, pp_link);
  /* Step 2: Initialize this page with zero.
   * Hint: use `memset`. */
  /* Exercise 2.4: Your code here. (2/2) */
  memset((void *)page2kva(pp), 0, BY2PG);
  *new = pp;
  return 0;
}
```

#### `page_decref()`

- 函数作用是通过页控制块改变页框的引用次数，若出现空闲页则调用回收函数`page_free()`回收内存

#### `page_free()` - Exercise 2.5

- ~~非常好水题，多来点（~~

- 任务要求为完成`page_free()`函数，该函数回收空闲页面（判定方式为`pp_ref == 0`），插入`page_free_list`中
- 提示：使用链表宏`LIST_INSERT_HEAD`，将页结构体插入空闲页结构体链表

```c
void page_free(struct Page *pp) {
  assert(pp->pp_ref == 0);
  /* Just insert it into 'page_free_list'. */
  /* Exercise 2.5: Your code here. */
  LIST_INSERT_HEAD(&page_free_list, pp, pp_link);
}
```



至此，Exercise 2.1-2.5顺利结束，**物理内存管理**模块练习完成



### 虚拟内存管理

在开始后续练习前，我们先了解一下实验环境中的虚拟地址分配：

#### 虚拟内存结构

- 32位虚拟内存：`PDX(va)|PTX(va)|offset (10 + 10 + 12)`
- **页表项结构：**
  - **一级页表项`Pde`，`Pde * = u_long = 物理页号|权限位`**
  - **二级页表项`Pte`，`Pte * = u_long = 物理页号|权限位`**
  - **权限位与`EntryLo`寄存器的标志位排布一致：`N|D|V|G|0[7:0]`，具体内容我们放到后面的`TLB`部分再详细说**
- **二级页表系统的作用：虚拟地址$\to$物理地址**
- **一/二级页表偏移量（`PDX(va)/PTX(va)`）：从一/二级页表基地址出发，移动n个单位到达**
- **一级页表项包含的页号是其对应的二级页表页的物理页号；**
- **二级页表项包含的页号是虚拟地址所对应的物理页页号**

> 访问虚拟地址时，先通过一级页表基地址和一级页表项的偏移量，找到对应的一级页表项，得到对应的二级页表的物理页号，再根据二级页表项的偏移量找到所需的二级页表项，进而得到该虚拟地址对应的物理页号

- 这个过程中需要将二级页表的物理页转换为物理地址，再转换为虚拟地址返回：`*pgdir -> PTE_ADDR(*pgdir) -> KADDR(PTE_ADDR(*pgdir))`，`Pde* pgdir`是偏移后得到的一级页表项

#### 一些小tips

- `kseg0/kseg1`使用`PADDR/KADDR`进行物理/虚拟地址的转换，而`kuseg`使用`TLB`（实际上是页表项）进行转换

- 在创建页表时，可以使用`KADDR`进行转换（处于内核态 存疑 通过`kseg0`读取）

- `memset`等函数操控的都是虚拟内存，不能直接访问物理内存

- **页表项与页控制块**

  - 页控制块是内存管理最开始初始化的`pages`变量，它管理物理页面的属性（前驱后继、引用次数等）

  - 页表项是虚拟页管理中最小的映射单元，它管理虚拟页面映射相关的属性（有效与否等）
  
  - 页控制块\to页表项的高位值：`*pte = page2pa(pp)`
  
  - 页表项\to页控制块：`pp = pa2page(*pte)`


```c
// include/mmu.h

#define BY2PG 4096							// 定义了页面字节数
#define PDMAP (4 * 1024 * 1024)				// 定义了一个一级页表页能管理的字节数
#define PGSHIFT 12							// 移动获得二级页号
#define PDSHIFT 22							// 移动获得一级页号
#define PDX(va) ((((u_long)(va)) >> 22) & 0x03ff)	// 获取31-22位，一级页表偏移量
#define PTX(va) ((((u_long)(va)) >> 12) & 0x03ff)	// 获取21-12位，二级页表偏移量
#define PTE_ADDR(pte) ((u_long)(pte) & ~0xfff)		// 获得对应的页表基址或者物理地址基址(低12位为0)

#define PPN(va) ((u_long)(va) >> 12)				// 地址对应的物理页号
#define VPN(va) ((u_long)(va) >> 12)				// 地址对应的虚拟页号
```

#### `pgdir_walk()` - Exercise 2.6

- 该函数的作用是：给定一个虚拟地址，在给定的页目录中查找这个虚拟地址对应的物理地址

  - 如存在：返回页表项所在的**虚拟**地址

  - 如不存在：连对应页表项都不存在（PDX指向的**页目录项**为**空或无效**），则根据传入的参数进行创建二级页表，或返回空指针。

- 注意，这里可能会在页目录表项无效且`create` 为真的情况，需要使用`page_alloc` 创建一个二级页表页，并应维护申请得到的物理页的`pp_ref` 字段
  - create = 0 ：纯查询功能

  - create = 1 ：新建二级页表项



（查物理地址）

```c
static int pgdir_walk(Pde *pgdir, u_long va, int create, Pte **ppte) {
  Pde *pgdir_entryp;
  struct Page *pp;

  /* Exercise 2.6: Your code here. (1/3) */
  pgdir_entryp = pgdir + PDX(va);

  /* Exercise 2.6: Your code here. (2/3) */
  if ((*pgdir_entryp & PTE_V) == 0) { // not existent
    if (create) {
      if (-E_NO_MEM == page_alloc(&pp)) {			// fail to collect a page to Page *pp
        return -E_NO_MEM;
      }
      pp->pp_ref++;
      *pgdir_entryp = page2pa(pp) | PTE_V | PTE_D;	// set Physical address of pp to it, and init 页表项
    } else {
      *ppte = NULL;									// no need to create a page
      return 0;
    }
  }
  /* Exercise 2.6: Your code here. (3/3) */
  // Pte *pte_location = (Pte *)(*pgdir_entryp >> 12 << 12) + PTX(va);
  // physical
  Pte *pte_location = (Pte *)(KADDR(PTE_ADDR(*pgdir_entryp))) +
                      PTX(va); // get pte virtual address
  *ppte = (pte_location);							
  return 0;
}

```

`(*pgdir_entryp & PTE_V)`是为了获取把目标一级页表项的有效位（使用位运算）

`(KADDR(PTE_ADDR(*pgdir_entryp)))`**获取一级页表项对应的二级页表的虚拟地址**，后面的加法表示二级页表偏移量，获得的是二级页表项地址



#### `page_insert()` - Exercise 2.7

- 该函数的作用是将一级页表基地址`pgdir`中虚拟地址`va`所在**虚拟页面**指向页控制块`pp`对应的**物理页面**，并将页表项权限为设置为`perm`。

（改变页表映射的得到的物理页面）

```c
int page_insert(Pde *pgdir, u_int asid, struct Page *pp, u_long va,u_int perm) {
    Pte *pte;

    /* Step 1: Get corresponding page table entry. */
    pgdir_walk(pgdir, va, 0, &pte);
    if (pte && (*pte & PTE_V)) {			// 页表项有效
        if (pa2page(*pte) != pp) {			// 指向的物理页不同，删除页表项
            page_remove(pgdir, asid, va);
        } else {
            tlb_invalidate(asid, va);			// 物理页相同，删除过时的 tlb 映射
            *pte = page2pa(pp) | perm | PTE_V;	// 更新为 pp 所在物理页并置有效位
            return 0;
        }
    }
    /* Step 2: Flush TLB with 'tlb_invalidate'. */
    /* Exercise 2.7: Your code here. (1/3) */
    tlb_invalidate(asid, va);					// 无效化 va 所在的 tlb 表项

    /* Step 3: Re-get or create the page table entry. */
    /* If failed to create, return the error. */
    /* Exercise 2.7: Your code here. (2/3) */
    int temp = pgdir_walk(pgdir, va, 1, &pte);
    if (-E_NO_MEM == temp) {
        return -E_NO_MEM;
    }

    /* Step 4: Insert the page to the page table entry with 'perm | PTE_V' and
     * increase its 'pp_ref'. */
    /* Exercise 2.7: Your code here. (3/3) */
    *pte = page2pa(pp) | perm | PTE_V;			// 同上，更新为 pp 所在物理页并置有效位
    pp->pp_ref++;
    return 0;
}

```

- `pa2page(*pte)`：取得二级页表项具体值，并取高位（物理页号）转换为`page`类指针，这里就是取得虚拟地址指的物理页的**page**结构体
- `page2pa(pp)|perm|PTE_V`：pp对应的物理地址基址（物理页号，低位为0），并增添`perm`与有效位
- 第一处walk，create=0，仅起到查询作用；第二处create=1，需要保证返回时pte对应物理页面

#### `page_lookup()`

- 该函数的作用是返回虚拟地址`va`映射物理页的页控制块`Page`，并将`ppte`指向的空间设为**二级页表项**地址

（查va对应的Page和Pte）

```c
struct Page *page_lookup(Pde *pgdir, u_long va, Pte **ppte) {
    struct Page *pp;
    Pte *pte;
    
    pgdir_walk(pgdir, va, 0, &pte);			// 获取 va 对应的二级页表项
    if (!(pte && (*pte & PTE_V))) {			// 页表项无效，返回
        return NULL;
    }
    
    pp = pa2page(*pte);						// 由 *pte 取出对应物理页块
    if (ppte) {
        *ppte = pte;						// 写入二级页表项地址
    }
    return pp;								// 返回页控制块
}
```



#### `page_remove()`

- 删除虚拟地址`va`在指定程序中所对应的**物理页面**映射

```c
void page_remove(Pde *pgdir, u_int asid, u_long va) {
    Pte *pte;
    struct Page *pp;
    pp = pgdir(pgdir, va, &pte);		// 查找对应的页控制块
    if (pp == NULL) {
        return;							// 没有映射，返回
    }
    page_decref(pp);					// 减少物理页引用次数
    *pte = 0;							// 清楚原有页表映射
    tlb_invalidate(asid, va);			// 无效化 tlb 中的页表映射
    return;
}
```



### 虚拟内存与 TLB

#### CP0 寄存器 - EntryHi、EntryLo、Index

- CP0 寄存器 EntryHi 与 EntryLo 分别对应到 TLB 的 Key 与 Data ，是 TLB 映射信息的来源
- ASID 与**虚拟页号**位于 EntryHi 中
- 有效位、可写位等标记位与**物理页号**在 EntryLo 中
- `Index`寄存器中存放访问的 TLB 的标号
- TLB 相当于一小段常用的**直接映射页表**，使用虚拟页号 + ASID 直接得到物理页号及相关信息

软件通过使用 CP0 寄存器和下面的汇编指令，把两个寄存器中分别存放的页号形成映射，存入 TLB 内部

#### TLB 相关汇编指令

`tlbr`：将`Index`的值作为索引，将当前索引指向的 TLB 表项**读出**至 CP0 寄存器

`tibwi`：将 CP0 寄存器的值回写入 TLB 索引的表项内

`tlbwr`：**randomly** 写入 TLB 表项内

`tlbp`：根据 EntryHi 查询 TLB 内的表项，并将查找到的索引存入 Index 内（若不存在则将 Index 最高位置为1）

#### TLB 维护与更新

1. 更新页表中虚拟地址对应的页表项的同时，将 TLB 中对应的**旧表项无效化**
2. 在下一次访问该虚拟地址时触发 **TLB 重填异常**，对 TLB 进行重填

#### `tlb_invalidate()` - Exercise 2.8

填空上毫无难度，汇编函数相关过程需要进行一些思考，详细可以参考 Thinking 2.5内容



#### `do_tlb_refill()` - Exercise 2.9 ~ 2.10

2.10类似于2.8，填空无难度，2.9的`do_tlb_refill`函数机制如下：

1. 进入函数，取出`TLB`缺失的**虚拟地址**（`BADVADDR`）和当前进程的`asid`（`EntryHi[11:6]`）
1. 调用C语言函数 Exercise 2.10 的`_do_tlb_refill()`（`jal _do_tlb_refill`），获取虚拟地址对应的**页表项**（`Pte *`）
1. 填空指令`tlbwr`随机地把保存在`EntryHi`和`EntryLo`中的信息写入一条`TLB`中

```c
Pte _do_tlb_refill(u_long va, u_int asid) {
  Pte *pte;
  /* Hints:
   *  Invoke 'page_lookup' repeatedly in a loop to find the page table entry
   * 'pte' associated with the virtual address 'va' in the current address space
   * 'cur_pgdir'.
   *
   *  **While** 'page_lookup' returns 'NULL', indicating that the 'pte' could
   * not be found, allocate a new page using 'passive_alloc' until 'page_lookup'
   * succeeds.
   */

  /* Exercise 2.9: Your code here. */
  // Page *pp = page_lookup(cur_pgdir, va, &pte);
  while (page_lookup(cur_pgdir, va, &pte) == NULL) {	// 查询 va 的页表项和物理页
    passive_alloc(va, cur_pgdir, asid);					// 调用被动分配函数分配物理页
  }
  return *pte;
}


static void passive_alloc(u_int va, Pde *pgdir, u_int asid) {
    struct Page *p = NULL;

    if (va < UTEMP) {
    	#define UTEMP (UCOW - BY2PG) = 4 * 1024 * 1024 - 2 * 1024
        panic("address too low");
    }

    if (va >= USTACKTOP && va < USTACKTOP + BY2PG) {
        #define USTACKTOP (UTOP - 2 * BY2PG) = 0x80000000 - 5 * 4 * 1024 * 1024
        panic("invalid memory");
    }

    if (va >= UENVS && va < UPAGES) {
        panic("envs zone");
    }

    if (va >= UPAGES && va < UVPT) {
        panic("pages zone");
    }

    if (va >= ULIM) {
        panic("kernel address");
    }

    panic_on(page_alloc(&p));
    panic_on(page_insert(pgdir, asid, p, PTE_ADDR(va), PTE_D));
}

#define KERNBASE 0x80010000

#define KSTACKTOP (ULIM + PDMAP)
#define ULIM 0x80000000

#define UVPT (ULIM - PDMAP)
#define UPAGES (UVPT - PDMAP)
#define UENVS (UPAGES - PDMAP)

#define UTOP UENVS
#define UXSTACKTOP UTOP

#define USTACKTOP (UTOP - 2 * BY2PG)
#define UTEXT PDMAP
#define UCOW (UTEXT - BY2PG)
#define UTEMP (UCOW - BY2PG)

```







## 附录



### `SLIST`宏代码

```c
/*
 * Singly-linked List definitions.
 */
#define	SLIST_HEAD(name, type)									\
struct name {													\
	struct type *slh_first;	/* first element */					\
}

#define	SLIST_HEAD_INITIALIZER(head)							\
	{ NULL }

#define	SLIST_ENTRY(type)										\
struct {														\
	struct type *sle_next;	/* next element */					\
}

/*
 * Singly-linked List functions.
 */

#define	SLIST_INIT(head) do {									\
	(head)->slh_first = NULL;									\
} while (/*CONSTCOND*/0)
    
#define	SLIST_INSERT_AFTER(slistelm, elm, field) do {			\
	(elm)->field.sle_next = (slistelm)->field.sle_next;			\
	(slistelm)->field.sle_next = (elm);							\
} while (/*CONSTCOND*/0)
    
#define	SLIST_INSERT_HEAD(head, elm, field) do {				\
	(elm)->field.sle_next = (head)->slh_first;					\
	(head)->slh_first = (elm);									\
} while (/*CONSTCOND*/0)
    
#define	SLIST_REMOVE_HEAD(head, field) do {						\
	(head)->slh_first = (head)->slh_first->field.sle_next;		\
} while (/*CONSTCOND*/0)
    
#define	SLIST_REMOVE(head, elm, type, field) do {				\
	if ((head)->slh_first == (elm)) {							\
		SLIST_REMOVE_HEAD((head), field);						\
	}															\
	else {														\
		struct type *curelm = (head)->slh_first;				\
		while(curelm->field.sle_next != (elm))					\
			curelm = curelm->field.sle_next;					\
		curelm->field.sle_next =								\
		    curelm->field.sle_next->field.sle_next;				\
	}															\
} while (/*CONSTCOND*/0)
    
#define	SLIST_FOREACH(var, head, field)							\
	for((var) = (head)->slh_first; (var); (var) = (var)->field.sle_next)
    
/*
 * Singly-linked List access methods.
 */
    
#define	SLIST_EMPTY(head)	((head)->slh_first == NULL)
#define	SLIST_FIRST(head)	((head)->slh_first)
#define	SLIST_NEXT(elm, field)	((elm)->field.sle_next)
```

