CROSS = arm-none-eabi-
CC = gcc
AS = as
ASFLAGS = -mcpu=arm1176jzf-s
CFLAGS = -O2 -Wall -mcpu=arm1176jzf-s -marm -nostartfiles -ffreestanding -nostdlib


LINKER_SCRIPT = kernel.ld

all:	kernel.img kernel.dis

kernel.img:	kernel.elf
	$(CROSS)objcopy kernel.elf -O binary kernel.img



kernel.elf:	kernel_main.o atags.o serial.o boot.o framebuffer.o \
	framebuffer_console.o idle_task.o interrupts.o io.o led.o \
	mailbox.o memory.o printk.o scheduler.o string.o syscalls.o time.o \
	timer.o 
	$(CROSS)ld --no-undefined \
		kernel_main.o atags.o serial.o boot.o framebuffer.o \
		framebuffer_console.o idle_task.o \
		interrupts.o io.o led.o mailbox.o memory.o \
		printk.o scheduler.o string.o syscalls.o time.o timer.o \
		-Map kernel.map -o kernel.elf -T $(LINKER_SCRIPT)


kernel_main.o:	kernel_main.c
	$(CROSS)$(CC) $(CFLAGS) -o kernel_main.o -c kernel_main.c


atags.o:	atags.c
	$(CROSS)$(CC) $(CFLAGS) -o atags.o -c atags.c

idle_task.o:	idle_task.c
	$(CROSS)$(CC) $(CFLAGS) -o idle_task.o -c idle_task.c

interrupts.o:	interrupts.c
	$(CROSS)$(CC) $(CFLAGS) -o interrupts.o -c interrupts.c

framebuffer.o:	framebuffer.c
	$(CROSS)$(CC) $(CFLAGS) -o framebuffer.o -c framebuffer.c

framebuffer_console.o:	framebuffer_console.c
	$(CROSS)$(CC) $(CFLAGS) -o framebuffer_console.o -c framebuffer_console.c

io.o:	io.c
	$(CROSS)$(CC) $(CFLAGS) -o io.o -c io.c

led.o:	led.c
	$(CROSS)$(CC) $(CFLAGS) -o led.o -c led.c

mailbox.o:	mailbox.c
	$(CROSS)$(CC) $(CFLAGS) -o mailbox.o -c mailbox.c

memory.o:	memory.c
	$(CROSS)$(CC) $(CFLAGS) -o memory.o -c memory.c

printk.o:	printk.c
	$(CROSS)$(CC) $(CFLAGS) -o printk.o -c printk.c

scheduler.o:	scheduler.c scheduler.h
	$(CROSS)$(CC) $(CFLAGS) -o scheduler.o -c scheduler.c

serial.o:	serial.c bcm2835_periph.h
	$(CROSS)$(CC) $(CFLAGS) -o serial.o -c serial.c

string.o:	string.c
	$(CROSS)$(CC) $(CFLAGS) -o string.o -c string.c

syscalls.o:	syscalls.c
	$(CROSS)$(CC) $(CFLAGS) -o syscalls.o -c syscalls.c

time.o:	time.c time.h
	$(CROSS)$(CC) $(CFLAGS) -o time.o -c time.c

timer.o:	timer.c
	$(CROSS)$(CC) $(CFLAGS) -o timer.o -c timer.c

boot.o:	boot.s
	$(CROSS)as $(ASFLAGS) -o boot.o boot.s




kernel.dis:	kernel.elf
	$(CROSS)objdump --disassemble-all kernel.elf > kernel.dis

clean:
	rm -f *~ *.o *.map *.elf kernel.img *.dis
	cd userspace && make clean

submit:
	tar -czvf hw3_submit.tar.gz *.c *.h *.s *.ld Makefile
