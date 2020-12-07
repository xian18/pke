#include <stdio.h>
void fun(int num){
	if(num==0){
		return;
	}
	fun(num-1);
}
int main(){
	int num=10000;
	fun(num);
	printf("end  \n");
	return 0;
}