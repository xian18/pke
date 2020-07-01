#include<stdio.h>

#define test_csrw() ({ \
  asm volatile ("csrw sscratch, 0"); })

int main()
{
	printf("user mode test illegal instruction!\n");
	// 在用户态调用内核态的指令属于非法，会引发非法指令错误！
	test_csrw();
	return 0;
}