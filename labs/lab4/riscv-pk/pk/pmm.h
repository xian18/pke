#include "list.h"
#include <stddef.h>
#include "encoding.h"
int pmm_init();
int pk_default_pmm_alloc();

struct Page {
    sint_t ref;                        // page frame's reference counter
    uint_t flags;                 // array of flags that describe the status of the page frame
    uint_t property;          // the num of free block, used in first fit pm manager
    list_entry_t page_link;         // free list link
};


// pmm_manager is a physical memory management class. A special pmm manager -
// XXX_pmm_manager
// only needs to implement the methods in pmm_manager class, then
// XXX_pmm_manager can be used
// by ucore to manage the total physical memory space.
struct pmm_manager {
    const char *name;  // XXX_pmm_manager's name
    void (*init)(
        void);  // initialize internal description&management data structure
                // (free block list, number of free block) of XXX_pmm_manager
    void (*init_memmap)(
        struct Page *base,
        size_t n);  // setup description&management data structcure according to
                    // the initial free physical memory space
    struct Page *(*alloc_pages)(
        size_t n);  // allocate >=n pages, depend on the allocation algorithm
    void (*free_pages)(struct Page *base, size_t n);  // free >=n pages with
                                                      // "base" addr of Page
                                                      // descriptor
                                                      // structures(memlayout.h)
    size_t (*nr_free_pages)(void);  // return the number of free pages
};
const struct pmm_manager *pmm_manager;


/* free_area_t - maintains a doubly linked list to record free (unused) pages */
typedef struct {
    list_entry_t free_list;         // the list header
    unsigned int nr_free;           // # of free pages in this free list
} free_area_t;


static inline void set_page_ref(struct Page *page, int val) { page->ref = val; }

// convert list entry to page
#define le2page(le, member)                 \
    to_struct((le), struct Page, member)

const struct pmm_manager default_pmm_manager;


size_t nbase;
extern struct Page *pages;
static inline struct Page *pa2page(uintptr_t pa) {
    return &pages[((uintptr_t)pa >> RISCV_PGSHIFT) - nbase];
}


static inline ppn_t page2ppn(struct Page *page) { return page - pages + nbase; }
static inline uintptr_t page2pa(struct Page *page) {
    return page2ppn(page) << RISCV_PGSHIFT;
}

