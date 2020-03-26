#include "console.h"
#include <stdint.h>
#include "defs.h"
#include "list.h"
#include "pmm.h"
#include "encoding.h"
#include "mmap.h"
#include "bits.h"

uintptr_t __page_alloc();

struct Page *pages;
extern uintptr_t first_free_paddr;
extern uintptr_t mem_size;
extern uintptr_t first_free_page;
int pmm_init(){
	pages= (struct Page *) __page_alloc();

   	nbase = ROUNDUP(first_free_paddr,RISCV_PGSIZE)>>RISCV_PGSHIFT;	
    pmm_manager = &default_pmm_manager;
    pmm_manager->init();
    pmm_manager->init_memmap(pa2page(first_free_paddr), mem_size/RISCV_PGSIZE);

	return 0;
}
int pk_default_pmm_alloc(){
	return 0;
}


free_area_t free_area;

#define free_list (free_area.free_list)
#define nr_free (free_area.nr_free)

//default pmm_manager

static void
default_init(void) {
    list_init(&free_list);
    nr_free = 0;
}


static void
default_init_memmap(struct Page *base, size_t n) {
 //   assert(n > 0);
    struct Page *p = base;
    for (; p != base + n; p ++) {
      //  assert(PageReserved(p));
        p->flags = p->property = 0;
        set_page_ref(p, 0);
    }
    base->property = n;
//    SetPageProperty(base);
    nr_free += n;
    if (list_empty(&free_list)) {
        list_add(&free_list, &(base->page_link));
    } else {
        list_entry_t* le = &free_list;
        while ((le = list_next(le)) != &free_list) {
            struct Page* page = le2page(le, page_link);
            if (base < page) {
                list_add_before(le, &(base->page_link));
                break;
            } else if (list_next(le) == &free_list) {
                list_add(le, &(base->page_link));
            }
        }
    }
}


static struct Page *
default_alloc_pages(size_t n) {
//    assert(n > 0);
    if (n > nr_free) {
        return NULL;
    }
    struct Page *page = NULL;
    list_entry_t *le = &free_list;
    while ((le = list_next(le)) != &free_list) {
        struct Page *p = le2page(le, page_link);
        if (p->property >= n) {
            page = p;
            break;
        }
    }
    if (page != NULL) {
        list_entry_t* prev = list_prev(&(page->page_link));
        list_del(&(page->page_link));
        if (page->property > n) {
            struct Page *p = page + n;
            p->property = page->property - n;
 //           SetPageProperty(p);
            list_add(prev, &(p->page_link));
        }
        nr_free -= n;
   //     ClearPageProperty(page);
    }
    return page;
}


static void
default_free_pages(struct Page *base, size_t n) {
//    assert(n > 0);
    struct Page *p = base;
    for (; p != base + n; p ++) {
  //      assert(!PageReserved(p) && !PageProperty(p));
        p->flags = 0;
        set_page_ref(p, 0);
    }
    base->property = n;
  //  SetPageProperty(base);
    nr_free += n;

    if (list_empty(&free_list)) {
        list_add(&free_list, &(base->page_link));
    } else {
        list_entry_t* le = &free_list;
        while ((le = list_next(le)) != &free_list) {
            struct Page* page = le2page(le, page_link);
            if (base < page) {
                list_add_before(le, &(base->page_link));
                break;
            } else if (list_next(le) == &free_list) {
                list_add(le, &(base->page_link));
            }
        }
    }

    list_entry_t* le = list_prev(&(base->page_link));
    if (le != &free_list) {
        p = le2page(le, page_link);
        if (p + p->property == base) {
            p->property += base->property;
    //        ClearPageProperty(base);
            list_del(&(base->page_link));
            base = p;
        }
    }

    le = list_next(&(base->page_link));
    if (le != &free_list) {
        p = le2page(le, page_link);
        if (base + base->property == p) {
            base->property += p->property;
      //      ClearPageProperty(p);
            list_del(&(p->page_link));
        }
    }
}

static size_t
default_nr_free_pages(void) {
    return nr_free;
}

const struct pmm_manager default_pmm_manager = {
    .name = "default_pmm_manager",
    .init = default_init,
    .init_memmap = default_init_memmap,
    .alloc_pages = default_alloc_pages,
    .free_pages = default_free_pages,
    .nr_free_pages = default_nr_free_pages,
};

