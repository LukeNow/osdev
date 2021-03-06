#ifndef _PROCESSOR_H
#define _PROCESSOR_H

#include "../include/kdef.h"

#define INT_SET 1
#define INT_CLEARED 0

/* TODO do we change all types to register_t? */

struct int_state {
	unsigned int cr0;
	unsigned int cr2;
	unsigned int cr3;
        unsigned int esp;
	unsigned int ebp;
	unsigned int edi;
	unsigned int esi;
	unsigned int edx;
	unsigned int ecx;
	unsigned int ebx;
	unsigned int eax;
	
	unsigned int int_num;
	unsigned int error_code;

	unsigned int eip;
	unsigned int cs;
	unsigned int eflags;
} __attribute__((packed));
typedef struct int_state int_state_t;

struct cpu_state {
	unsigned int cr0;
	unsigned int cr2;
	unsigned int cr3;
	unsigned int eflags;
	unsigned int eip;
        unsigned int esp;
	unsigned int ebp;
	unsigned int edi;
	unsigned int esi;
	unsigned int edx;
	unsigned int ecx;
	unsigned int ebx;
	unsigned int eax;
} __attribute__((packed));
typedef struct cpu_state cpu_state_t;

void __halt_system();
void __enable_int();
void __disable_int();
void __nop();

unsigned int get_int();
unsigned int get_cr3();
unsigned int get_cr2();
unsigned int get_cr0();
unsigned int get_eax();
unsigned int get_ebx();
unsigned int get_ecx();
unsigned int get_edx();
unsigned int get_esi();
unsigned int get_edi();
unsigned int get_ebp();
unsigned int get_esp();
unsigned int get_eip();
unsigned int get_eflags();

#endif
