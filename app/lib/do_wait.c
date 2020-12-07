
int do_wait(int pid){
	int num=3;
	int ret;
    asm volatile (
        "lw x17, %1\n"
	"lw x10,%2\n"
        "ecall\n"
        "sd a0, %0"
        : "=m" (ret)
        : "m" (num),"m"(pid)
        : "memory"
      );
    return ret;
}


