#include<stdio.h>
#include "libuser.h"


#define N 2

typedef int semaphore;

semaphore mutex = 1;

semaphore empty = N;

semaphore full = 0;

int items=0;
void produce_item(){
}

void insert_item(){
    items++;
    printf("printf 5_2 produce one  item ,father  total procude %d  \n",items);
}

void remove_item(){
    items++;
    int a=1;
     for(int i=0;i<10000000;i++){  //模拟消耗时间
     
         a=a*i+1;
     }

    printf("printf 5_2 consume one  item,child %d total consume %d  random %x\n",getpid(),items,a);
}

void consume_item(){
   
 
}

void producer(void)

{



	while(1)

	{
        if(items==5*N) break;
		produce_item();

		down(&empty);				//空槽数目减1，相当于P(empty)

		down(&mutex);				//进入临界区，相当于P(mutex)

		insert_item();			//将新数据放到缓冲区中

		up(&mutex);				//离开临界区，相当于V(mutex)

		up(&full);				//满槽数目加1，相当于V(full)

	}

}

void consumer()

{


	while(1)

	{

		down(&full);				//将满槽数目减1，相当于P(full)

		down(&mutex);				//进入临界区，相当于P(mutex)

		remove_item();	   		 //从缓冲区中取出数据

		up(&mutex);				//离开临界区，相当于V(mutex)		

		up(&empty);				//将空槽数目加1 ，相当于V(empty)

		consume_item();			//处理取出的数据项

	}

}

int main(){
    int  pid;

    int i;

    for(i=0; i<2; i++){
        pid=fork();
        if(pid==0||pid==-1)  //子进程或创建进程失败均退出
        {
            break;
        }
    }
    if(pid==0) {
        // 
        printf("printf 5_2 child %d\n",getpid());
        consumer(getpid());      
        
    }else{
        // 
        printf("printf 5_2 father %d \n",getpid());
        producer();
    
    }

    return 0;
}