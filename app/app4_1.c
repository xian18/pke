#include<stdio.h>

int main()
{

	uintptr_t addr = 0x7f000000;
	*(int *)(addr)=1;

	uintptr_t addr1_same_page = 0x7f000010;
	uintptr_t addr2_same_page = 0x7f000fff;
	*(int *)(addr1_same_page)=2;
	*(int *)(addr2_same_page)=3;

	uintptr_t addr1_another_page = 0x7f001000;
	uintptr_t addr2_another_page = 0x7f001ff0;
	*(int *)(addr1_another_page)=4;
	*(int *)(addr2_another_page)=5;

	
	return 0;
}
