#include <stddef.h>
#include <stdint.h>
#include "printk.h"
#include "string.h"
#include "memory.h"
#include "scheduler.h"


int current_process=0;
struct process_control_block_type process[MAX_PROCESSES];
static int avail_pid=0;


int processes_init(void) {
	int i;

	current_process=0;
	for(i=0;i<MAX_PROCESSES;i++) {
		process[i].valid=0;
	}

	return 0;
}

int load_process(char *name,
		unsigned char *data, int size, unsigned int stack_size) {

	char *binary_start;
	char *stack_start;
	int i,j;

	/* LOCK */

	/* Find free process */
	for(i=0;i<MAX_PROCESSES;i++) {
		if (!process[i].valid) break;
	}

	if (i==MAX_PROCESSES) {
		printk("ERROR: No free process slot!\n");
		/* UNLOCK */
		return -1;
	}

	process[i].valid=1;

	/* UNLOCK */

	/* Allocate Memory */
        binary_start=(char *)memory_allocate(size);
        stack_start=(char *)memory_allocate(stack_size);


	/* Load executable */
	printk("Copying %d bytes from %x to %x\r\n",size,data,binary_start);
        memcpy(binary_start,data,size);

	/* Set name */
	strncpy(process[i].name,name,32);

	/* Set up initial conditions */
	process[i].running=0;
	process[i].ready=1;
	process[i].time=0;
	/* LOCK */
	process[i].pid=avail_pid;
	avail_pid++;
	/* UNLOCK */

	/* Initialize register state */
	for(j=0;j<15;j++) {
		process[i].reg_state.r[j]=0;
	}

	/* Setup the stack */
	/* is the -4 needed? */
	process[i].reg_state.r[13]=((long)stack_start+stack_size);

	/* Setup the entry point */
	process[i].reg_state.lr=(long)binary_start;

	/* Setup the SPSR */
	/* USER mode (0x10) */
	/* We don't mask 0x80 or 0x40 (IRQ or FIQ) */
	process[i].reg_state.spsr=0x10;

        printk("New process %s pid %d "
		"allocated %dkB at %x and %dkB stack at %x\r\n",
		name,process[i].pid,
		size/1024,binary_start,
		stack_size/1024,stack_start);


	return i;
}

int run_process(int which) {

	long *our_sp;
	long return_pc,our_spsr;

	return_pc=process[which].reg_state.lr;
	our_spsr=process[which].reg_state.spsr;

	our_sp=&(process[which].reg_state.r[0]);

	printk("Attempting to run proc %d (%s pid=%d): "
		"PC=%x SPSR=%x save_addr=%x stack=%x\r\n",
		which, process[which].name,process[which].pid,
		return_pc,our_spsr,our_sp,
		process[which].reg_state.r[13]);

	/* restore user registers */
	/* update status */
	/* jump to saved user PC */

	current_process=which;

	asm volatile(
		"mov r0, %[our_sp]\n"
		"msr SPSR_cxsf, %[our_spsr]\n"
		"mov lr, %[return_pc]\n"
		"ldmia r0, {r0 - lr}^\n"	/* the ^ means load user regs */
		"nop\n"
		/* Need to reset IRQ stack here or we leak */
		"movs pc, lr\n"			/* movs with pc changes mode */
		: /* output */
		:	[our_sp] "r"(our_sp),
			[return_pc] "r"(return_pc),
			[our_spsr] "r"(our_spsr) /* input */
		: "r0", "lr", "sp", "memory" /* clobbers */
			);


		printk("Should never get here!\r\n");
#if 0
	/* set user stack */
        asm volatile(
                "msr CPSR_c, #0xDF\n" /* System mode, like user but privldg */
                "mov sp, %[stack]\n"
                "msr CPSR_c, #0xD3\n" /* Back to Supervisor mode */
                : /* output */
                : [stack] "r"(shell_stack) /* input */
                : "sp", "memory");      /* clobbers */

        /* enter userspace */

        asm volatile(
                "mov r0, #0x10\n"
                "msr SPSR, r0\n"
                "mov lr, %[shell]\n"
                "movs pc,lr\n"
                : /* output */
                : [shell] "r"(shell_address) /* input */
                : "r0", "lr", "memory");        /* clobbers */
#endif

	return 0;
}

void schedule(void) {

	int i;

	i=current_process;

	/* save current process state */

	/* find next available process */
	/* Should we have an idle process (process 0) */
	/* That is special cased and just runs wfi?   */
	i++;

	while(!process[i].ready) {
		if (i>=MAX_PROCESSES) i=0;
		i++;
	}

	/* switch to new process */

	run_process(i);

}