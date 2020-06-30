#include<stdio.h>

static inline int syscall(int num){
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


    if(syscall(170) == 0) {
        printf("this is child process;my pid = %d\n",syscall(172));
    }else {
        printf("this is farther process;my pid = %d\n",syscall(172));
    }

    return 0;
}