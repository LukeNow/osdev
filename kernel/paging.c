#include <stdint.h>
#include <stddef.h>

#include "../include/paging.h"
#include "../include/kheap.h"
#include "../include/kprint.h"
#include "../include/string.h"

#define PAGE_SIZE 4096
#define MB(X) ((2 << 20) * (X))
#define DIR_SIZE 2 << 22
#define KERN_MAX_VIRTUAL_ADDR 0xFFFFFFFF
#define KERN_VIRTUAL_ADDR 0xC0000000

#define PAGE_DIRECTORY_INDEX(x) (((x) >> 22) & 0x3ff)
#define PAGE_TABLE_INDEX(x) (((x) >> 12) & 0x3ff)
#define GET_TBL_PHYS_ADDR(x) ((x) & ~0x3ff)
#define PAGE_GET_PHYS_ADDRR(x) (*x & ~0xfff)


extern void paging_init(page_dir_t * dir);

void set_page_dir_entry(page_dir_entry_t *entry, uint32_t phys_addr,
			 int read_write, int privledge, int write_through,
			 int cache_disabled, int page_size)
{
	uint32_t temp_entry = 0;
	uint32_t temp_addr = ALIGN_UP(phys_addr, PAGE_SIZE);

	temp_entry |= (temp_addr & ~0x03FF);
	temp_entry |= 1;
	temp_entry |= read_write << 1;
	temp_entry |= privledge << 2;
	temp_entry |= write_through << 3;
	temp_entry |= cache_disabled << 4;
	temp_entry |= page_size << 7;


	kprint(INFO, "DIR entry %x, phys_addr %x, entry %x\n", (uint32_t)entry, phys_addr, temp_entry);
	*entry = (page_dir_entry_t) temp_entry;
}


void set_page_tbl_entry(page_tbl_entry_t *entry, uint32_t phys_addr, 
			int read_write, int privledge, int write_through, 
			int cache_disabled)
{
	uint32_t temp_entry = 0;
	uint32_t temp_addr = ALIGN_UP(phys_addr, PAGE_SIZE);
	//kprint(INFO, "TEMP ADDR %x\n", temp_addr);
	temp_entry |= (phys_addr & ~0x03FF);
	temp_entry |= 1;
	temp_entry |= read_write << 1;
	temp_entry |= privledge << 2;
	temp_entry |= write_through << 3;
	temp_entry |= cache_disabled << 4;

	kprint(INFO, "PAGE entry %x, phys_addr %x, entry %x\n", 
			(uint32_t)entry, phys_addr, temp_entry);
	*entry = (page_tbl_entry_t) temp_entry;
}

static void map_kern_page(page_dir_t *dir, uint32_t virtual_addr, uint32_t phys_addr)
{
	unsigned long pdindex = virtual_addr >> 22;
	unsigned long ptindex = virtual_addr >> 12 & 0x03FF;

	if (dir->page_dir[pdindex] == 0) {
		/* Map new page table */
		unsigned long temp_phys_addr = 0;
		temp_phys_addr = early_kmalloc_pages(1);
		memset((uint32_t *)temp_phys_addr, 0, PAGE_SIZE);
		set_page_dir_entry((page_dir_entry_t*)&dir->page_dir[pdindex],
				   temp_phys_addr, 1, 0, 0, 0, 0);
	}
	

	kprint(INFO, "VIRTUAL ADDR: %x\n", virtual_addr); 
	page_tbl_t *tbl = (page_tbl_t *) GET_TBL_PHYS_ADDR(dir->page_dir[pdindex]);
	page_tbl_entry_t* tbl_entry= (page_tbl_entry_t *) &tbl->page_tbl[ptindex];
	set_page_tbl_entry(tbl_entry, phys_addr, 1, 0, 0, 0);
}


void init_kern_paging()
{
	unsigned long virt_addr_count = 0;
	uint32_t phys_addr_count = 0;
	uint32_t page_dir_addr_phys = 0;
	page_dir_t* kernel_dir; //physical_addr

	/* Allocate main kern directory */
	page_dir_addr_phys = early_kmalloc_pages(1);
	kernel_dir = (page_dir_t *)page_dir_addr_phys;
	memset(kernel_dir, 0, PAGE_SIZE);
	
	//kprint(INFO, "KERNEL DIR ADDR %x\n", kernel_dir);

	/* Identity map the first 1MB 
	 * PHYSICAL_ADDR = VIRTUAL_ADDR */
	while (phys_addr_count < MB(1)) {
		
		//kprint(INFO, "PHYS ADDR: %x, VIRT ADDR: %x\n", phys_addr_count, phys_addr_count);
		map_kern_page(kernel_dir, phys_addr_count, phys_addr_count);		
		phys_addr_count += PAGE_SIZE;
	}
	
	
	//PANIC("STOP");
	virt_addr_count = KERN_VIRTUAL_ADDR;
	phys_addr_count = 0;
	/* Map the from 1MB - 4MB -> 3GB + 1MB - 3GB + 4MB */
	while (phys_addr_count < MB(4)) {
		map_kern_page(kernel_dir, virt_addr_count, phys_addr_count);
		virt_addr_count += PAGE_SIZE;
		phys_addr_count += PAGE_SIZE;
	}
	kprint(INFO, "KERNEL ADDR: %x\n", kernel_dir);
	//PANIC("STOP");
	paging_init(kernel_dir);
}

