#include <stdint.h>


# if __riscv_xlen == 64
	typedef int64_t sint_t;
	typedef uint64_t uint_t;
#else
	typedet int32_t sint_t;
	typedet uint32_t uint_t;
#endif

typedef uint_t size_t;

typedef int bool;
/* to_struct - get the struct from a ptr
 * @ptr:    a struct pointer of member
 * @type:   the type of the struct this is embedded in
 * @member: the name of the member within the struct
 * */
#define to_struct(ptr, type, member)                               \
    ((type *)((char *)(ptr) - offsetof(type, member)))

typedef size_t ppn_t;

