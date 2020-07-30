#define ecall() ({\
  asm volatile(\
  	"li x17,81\n"\
    "ecall");\
})

int main(void){
	//调用自定义的81号系统调用
	ecall();
	return 0;
}

