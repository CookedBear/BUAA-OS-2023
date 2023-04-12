void mips_init() {
	printk("init.c:\tmips_init() is called\n");

	mips_detect_memory();
	mips_vm_init();
	page_init();
	env_init();
	printk("env_init success\n");

	struct Env *ppb = ENV_CREATE_PRIORITY(test_ppb, 5);
	struct Env *ppc = ENV_CREATE_PRIORITY(test_ppc, 5);
	ppc->env_parent_id = ppb->env_id;
	printk("ppb->envid: %d\n",ppb->env_id);
	printk("ppc->envid: %d\n",ppc->env_id);
	kclock_init();
	enable_irq();
	while (1) {
	}
	panic("init.c:\tend of mips_init() reached!");
}
