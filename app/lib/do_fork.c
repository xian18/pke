int do_fork(){
	int num=170;
	int ret;
    asm volatile (
        "lw x17, %1\n"
        "ecall\n"
        "sd a0, %0"
        : "=m" (ret)
        : "m" (num)
        : "memory"
      );
    return ret;
}
