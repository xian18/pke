#include<stdio.h>

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

int main(){


    if(fork() == 0) {
        printf("this is child process;my pid = %d\n",getpid());
    }else {
        printf("this is farther process;my pid = %d\n",getpid());
    }

    return 0;
}
