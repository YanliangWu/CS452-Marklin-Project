#
# Makefile for busy-wait IO tests
#
XCC     = gcc
AS	= as
LD      = ld
CFLAGS  = -c -fPIC -Wall -I. -I../include -mcpu=arm920t -msoft-float -g

# Optimization Flag
O2FLAGS = -c -O2 -fPIC -Wall -I. -I../include -mcpu=arm920t -msoft-float -g
# -g: include hooks for gdb
# -c: only compile
# -mcpu=arm920t: generate code for the 920t architecture
# -fpic: emit position-independent code
# -Wall: report all warnings

ASFLAGS	= -mcpu=arm920t -mapcs-32
# -mapcs: always generate a complete stack frame

LDFLAGS = -init main -Map main.map -N  -T orex.ld -L/u/wbcowan/gnuarm-4.0.2/lib/gcc/arm-elf/4.0.2 -L../lib

all:   main.elf main.s

string.s: string.c string.h
	$(XCC) -S $(CFLAGS) string.c

string.o: string.s
	$(AS) $(ASFLAGS) -o string.o string.s

uartserver.s: uartserver.c uartserver.h
	$(XCC) -S $(CFLAGS) uartserver.c

uartserver.o: uartserver.s
	$(AS) $(ASFLAGS) -o uartserver.o uartserver.s

print.s: print.c print.h
	$(XCC) -S $(CFLAGS) print.c

print.o: print.s
	$(AS) $(ASFLAGS) -o print.o print.s

initialization.s: initialization.c initialization.h
	$(XCC) -S $(CFLAGS) initialization.c

initialization.o: initialization.s
	$(AS) $(ASFLAGS) -o initialization.o initialization.s

bwio.s: bwio.c bwio.h
	$(XCC) -S $(CFLAGS) bwio.c

bwio.o: bwio.s
	$(AS) $(ASFLAGS) -o bwio.o bwio.s

kernel_hwi.s: kernel_hwi.c kernel_hwi.h
	$(XCC) -S $(O2FLAGS) kernel_hwi.c

kernel_hwi.o: kernel_hwi.s
	$(AS) $(ASFLAGS) -o kernel_hwi.o kernel_hwi.s

priority_queue.s: priority_queue.c priority_queue.h
	$(XCC) -S $(CFLAGS) priority_queue.c

priority_queue.o: priority_queue.s
	$(AS) $(ASFLAGS) -o priority_queue.o priority_queue.s

task.s: task.c task.h
	$(XCC) -S $(CFLAGS) task.c

task.o: task.s
	$(AS) $(ASFLAGS) -o task.o task.s

kernel_swi.s: kernel_swi.c kernel_swi.h
	$(XCC) -S $(CFLAGS) kernel_swi.c

kernel_swi.o: kernel_swi.s
	$(AS) $(ASFLAGS) -o kernel_swi.o kernel_swi.s

syscall.s: syscall.c syscall.h
	$(XCC) -S $(O2FLAGS) syscall.c

syscall.o: syscall.s
	$(AS) $(ASFLAGS) -o syscall.o syscall.s

track.s: track.c track.h
	$(XCC) -S $(CFLAGS) track.c

track.o: track.s
	$(AS) $(ASFLAGS) -o track.o track.s

nameserver.s: nameserver.c nameserver.h
	$(XCC) -S $(O2FLAGS) nameserver.c

nameserver.o: nameserver.s
	$(AS) $(ASFLAGS) -o nameserver.o nameserver.s

clockserver.s: clockserver.c clockserver.h
	$(XCC) -S $(O2FLAGS) clockserver.c

clockserver.o: clockserver.s
	$(AS) $(ASFLAGS) -o clockserver.o clockserver.s

trainserver.s: trainserver.c trainserver.h
	$(XCC) -S $(O2FLAGS) trainserver.c

trainserver.o: trainserver.s
	$(AS) $(ASFLAGS) -o trainserver.o trainserver.s

rps.s: rps.c rps.h
	$(XCC) -S $(CFLAGS) rps.c

rps.o: rps.s
	$(AS) $(ASFLAGS) -o rps.o rps.s

main.s: main.c main.h
	$(XCC) -S $(CFLAGS) main.c

main.o: main.s
	$(AS) $(ASFLAGS) -o main.o main.s

asm.o: asm.s
	$(AS) $(ASFLAGS) -o asm.o asm.s

main.elf: string.o bwio.o initialization.o uartserver.o print.o asm.o kernel_hwi.o priority_queue.o task.o kernel_swi.o syscall.o trainserver.o track.o nameserver.o clockserver.o rps.o main.o
	$(LD) $(LDFLAGS) -o $@ string.o bwio.o initialization.o uartserver.o print.o asm.o kernel_hwi.o priority_queue.o task.o kernel_swi.o syscall.o trainserver.o track.o nameserver.o clockserver.o rps.o main.o -lgcc

clean:
	-rm -f main.elf *.s *.o main.map
