// int down(int * va);
// int up(int * va);
// int fork();
// int getpid();
// int wait(int pid);

static inline int fork(){
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


static inline int getpid(){
	int num=172;
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




static inline int wait(int pid){
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
static inline int down(int * va){
	int num=82;
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


static inline int up(int * va){
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