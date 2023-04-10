- PDX(va) ：页目录偏移量（查找遍历页表时常用）
- PTX(va) ：页表偏移量（查找遍历页表时常用）
- PTE_ADDR(pte) ：获取页表项中的物理地址（读取 pte 时常用）
- PADDR(kva) ：kseg0 处虚地址 物理地址
- KADDR(pa) ：物理地址 kseg0 处虚地址（读取 pte 后可进行转换）
- va2pa(Pde *pgdir, u_long va) ：查页表，虚地址 物理地址（测试时常用）
- pa2page(u_long pa) ：物理地址 页控制块（读取 pte 后可进行转换）
- page2pa(struct Page *pp) ：页控制块 物理地址（填充 pte 时常用）

## lab3 错误点记录

第一个问题就是 Exercise 3.8 忘了写都能过 Test 3-1（）
然后等到写完 Test 3-4 的时候发现怎么改都过不了，逆天
其次是在 Exercise 3.11 中，不知道还能引用 `.h` 文件中的宏定义，导致表示地址的时候出了一些问题，正解应该是
```mipsasm
sw 		t0 , (KSEG1 | DEV_RTC_ADDRESS | DEV_RTC_HZ)
```
最后是 Exercise 3.12 中有好几个问题：
- `e->env_status != ENV_RUNNABLE` 与 `e->env_status == ENV_NOT_RUNNABLE` 两者意义不同，要求应该是非运行时进行替换，而不是处在**不运行**状态时替换，看起来很相近但是忽略了 `ENV_FREE` 其实也是非运行的一种
- 读注释不仔细，两个大错都是这里出的：
> set 'count' to its priority, and schedule it with 'env_run'. **Panic if that list is empty**.
- 首先是忘了 `env_run()` 这个新的进程，直接万劫不复（x
- 其次是没有输出 `Panic` 导致测评数据不太对