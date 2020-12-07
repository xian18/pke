int up(int * va){
	int num=83;
	int ret;
    asm volatile (
        "lw x17, %1\n"
        "lw x10 ,%2 \n"
        "ecall\n"
        "sd a0, %0\n"
        : "=m" (ret)
        : "m" (num), "m"(va)
        : "memory"
      );
    return ret;
}