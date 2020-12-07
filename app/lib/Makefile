app_objs=do_fork.o do_wait.o do_getpid.o down.o up.o

CC            := riscv64-unknown-elf-gcc
AR            := riscv64-unknown-elf-ar
RANLIB        := riscv64-unknown-elf-ranlib
CFLAGS        := -Wall -Werror -D__NO_INLINE__ -mcmodel=medany -O2 -std=gnu99 -Wno-unused -Wno-attributes -fno-delete-null-pointer-checks -fno-PIE 
COMPILE       := $(CC) -MMD -MP $(CFLAGS) 


libuser.a: $(app_objs)
	$(AR) rcv -o $@ $^
	$(RANLIB) $@



$(app_objs): %.o : %.c
	$(COMPILE) -c $< -o $@

clean:
	rm *.o *.d *.a