#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "../include/tty.h"
#include "../include/kprint.h"
#include "../include/machine.h"
#include "../include/processor.h"
#include "../include/io.h"
#include "../include/multiboot.h"
#include "../include/kheap.h"
#include "../include/string.h"
#include "../include/mm.h"
#include "../include/kdef.h"
#include "../include/paging.h"
#include "../include/sorted_array_list.h"


#define SECTOR_SIZE 512
#define ARRAY_EXPAND_SIZE (SECTOR_SIZE / sizeof(void*))

void kmain(multiboot_info_t* mbt, unsigned int magic)
{
	
	extern void heap_top();
	extern void heap_bottom();
	extern void stack_top();
	
	extern unsigned int pages_mapped;
	
	uint32_t mem_limit;
	int i = 0;
	int addr = 0;
	
	multiboot_memory_map_t* mmap = mbt->mmap_addr;
	
	kprint(INFO, "MEM SIZE: %x\n", mbt->mem_upper * 1024);
	mem_limit = mbt->mem_upper * 1024;
	init_early_kheap();
	init_mem_manager(mem_limit); //Physical page alloc
	init_kern_paging(); //Map kernel pages to table
	
	init_kheap(); //Regular kheap
	
	
	
	//kprint(INFO, "Phys pages mapped %d\n", pages_mapped);
	
	/*
	sorted_array_list_t list;
	addr = early_kmalloc_sectors(1);
	
	init_sorted_list(&list, (void**)addr, ARRAY_EXPAND_SIZE, 
			 expand_array, contract_array);

	
	
	
	int ret = insert_list(12, &list);
	ret = insert_list(11, &list);
	ret = insert_list(10, &list);	
	ret = insert_list(9, &list);	
	ret = insert_list(8, &list);	
	ret = insert_list(7, &list);	
	ret = insert_list(6, &list);	
	ret = insert_list(5, &list);	
	ret = insert_list(4, &list);	
	ret = insert_list(3, &list);	

	
	for (int i = 0; i < 15; i++) {
		kprint(INFO, "%d: %d\n", i, list.arr[i]);
	}
	
	ret = remove_list(3, &list);
	ret = remove_list(12, &list);
	for (int i = 0; i < 15; i++) {
		kprint(INFO, "%d: %d\n", i, list.arr[i]);
	}
	*/
	/*
	for (int i = 0; i < 513; i++) {
		ret = insert_list(i + 2, &list);
	}

	ret = insert_list(1, &list);
	ret = insert_list(0, &list);

	ret = remove_list(1, &list);

	ret = remove_list(2, &list);
	ret = remove_list(3, &list);
	ret = remove_list(4, &list);
	ret = remove_list(5, &list);
	ret = remove_list(6, &list);
	for (int i = 0; i < 513; i++) {
		kprint(INFO, "%d: %d\n", i, list.arr[i]);
	}
	*/
	
	/*
	uint32_t mem_count = 0;
	for(int i = 0; i < 100; i++) {
		uint32_t temp_addr = get_free_page_mem();
		
		//kprint(INFO, "temp_addr: %x\n", temp_addr);
		if (temp_addr != mem_count) {
			kprint(INFO, "NOT EQUAL ON %d\n", i);
			PANIC("SHIT");
		}
		mem_count += PAGE_SIZE;
	}
	
	mem_count = 0;
	for (int i = 0; i < 16; i++) {
		release_page_mem(mem_count);
		mem_count += PAGE_SIZE;
	}
	
	uint32_t temp_addr = get_free_page_mem();
	if (temp_addr != 0) {
		PANIC("WHOOPS1");
	}

	release_page_mem(mem_count);
	temp_addr = get_free_page_mem();
	if (temp_addr != 4096) {
		PANIC("WHOOPS2");
	}
	*/

	PANIC("KMAIN STOP");
	/* Hang, we dont return from this function */
	for(;;) 
	
	return;
}
