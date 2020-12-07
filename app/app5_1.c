#include<stdio.h>
#include "libuser.h"

int main(){
     int a=10;
     int ret=-1;
     printf("print proc app5_1\n");
     if((ret=do_fork()) == 0) {
        printf("print proc this is child process;my pid = %d\n",do_getpid());
       a=a-1;
	printf("print proc a=%d\n",a);
    }else { 
        int wait_ret = -1;
	wait_ret=do_wait(ret);	
    a=a-2;
	printf("print proc this is farther process;my pid = %d\n",do_getpid());
	printf("print proc a=%d\n",a);
    }

    if((ret=do_fork()) == 0) {
        a=a-3;
        printf("print proc this is child process;my pid = %d\n",do_getpid());
	printf("print proc a=%d\n",a);
    }else { 
        a=a-4;
        int wait_ret = -1;
	wait_ret=do_wait(ret);	
	printf("print proc this is farther process;my pid = %d\n",do_getpid());
	printf("print proc a=%d\n",a);
    }

    return 0;
}
