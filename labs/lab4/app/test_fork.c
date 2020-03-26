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
	printf("----------app----------\n");

    if(syscall(82) == 0) {
        printf("this is child porcess;my pid = %d\n",syscall(172));
    }else {
        printf("this is farther porcess;my pid = %d\n",syscall(172));
    }
    printf("----------over----------\n");
    return 0;
}