#include<stdio.h>

long unsigned int addr_line=0x7f7ea000;
int main()
{

        long unsigned int addr_u = 0x7f7ecc00;
        long unsigned int addr_m = 0x8f000000;

        //在用户模式下访问用户空间地址
        printf("APP: addr_u 0x%016lx\n",addr_u);
        *(int *)(addr_u)=1;

        //用户模式下访问内核空间地址，会引发段错误
        printf("APP: addr_m 0x%016lx\n",addr_m);
        *(int *)(addr_m)=1;

        return 0;
}
