OBJS_BOOTPACK = bootpack.o asmfunc.o hankaku.o graphic.o dsctbl.o int.o sprintf.o fifo.o

default:
	make img

ipl10.bin : ipl10.asm Makefile
	nasm ipl10.asm -o ipl10.bin -l ipl10.lst

asmhead.bin : asmhead.asm Makefile
	nasm asmhead.asm -o asmhead.bin -l asmhead.lst

asmfunc.o : asmfunc.asm Makefile
	nasm -f elf32 asmfunc.asm -o asmfunc.o

hankaku.o : hankaku.asm Makefile
	nasm -f elf32 hankaku.asm -o hankaku.o

bootpack.elf : $(OBJS_BOOTPACK) Makefile
	ld -m elf_i386 -e HariMain -Ttext 0x280000 $(OBJS_BOOTPACK) -o bootpack.elf
#	objcopy	-O binary bootpack.elf bootpack.bin
#	cp bootpack.elf bootpack.bin

haribote.sys : asmhead.bin bootpack.elf Makefile
	cat asmhead.bin bootpack.elf >  haribote.sys

haribote.img : ipl10.bin haribote.sys Makefile
	dd if=/dev/zero	of=haribote.img bs=1024 count=1440
	dd if=ipl10.bin of=haribote.img bs=512 count=1 conv=notrunc
	sudo mount haribote.img /mnt/floppy -o loop
	sudo cp haribote.sys /mnt/floppy
	sudo umount /mnt/floppy

%.o : %.c Makefile
	gcc -m32 -c -g -Wa,-adlhn -c $*.c > $*.anno.s
	gcc -m32 -fno-stack-protector -c $*.c -o $*.o
#
img :
	make -r haribote.img

asm :
	make -r ipl10.bin
	make -r asmhead.bin
	make -r bootpack.bin
	make -r harobote.sys

run :
	make img
	qemu-system-i386 -fda haribote.img -boot a -no-acpi

debug :
	bochs -q

umount :
	sudo umount /mnt/floppy

clean :
	rm *.bin
	rm *.lst
	rm *.o
	rm *.elf
	rm *.sys
	rm *.s
src_only :
	make clean
	make clean_~
	rm haribote.img bochsout.txt
clean_~:
	rm *~

clean_debug :
	rm bochsout.txt
