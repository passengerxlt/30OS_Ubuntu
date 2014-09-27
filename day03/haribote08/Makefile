default:
	make img

ipl10.bin : ipl10.asm Makefile
	nasm ipl10.asm -o ipl10.bin -l ipl10.lst

asmhead.bin : asmhead.asm Makefile
	nasm asmhead.asm -o asmhead.bin -l asmhead.lst

bootpack.bin : bootpack.c Makefile
	gcc -m32 -c bootpack.c -o bootpack.o
	ld -m elf_i386 bootpack.o -o bootpack.elf
	objcopy	-O binary bootpack.elf bootpack.bin

haribote.sys : asmhead.bin bootpack.bin Makefile
	cat asmhead.bin bootpack.bin >  haribote.sys

haribote.img : ipl10.bin haribote.sys Makefile
	dd if=/dev/zero	of=haribote.img bs=1024 count=1440
	dd if=ipl10.bin of=haribote.img bs=512 count=1 conv=notrunc
	sudo mount haribote.img /mnt/floppy -o loop
	sudo cp haribote.sys /mnt/floppy
	sudo umount /mnt/floppy

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
	rm ipl10.bin ipl10.lst asmhead.bin asmhead.lst bootpack.o bootpack.elf bootpack.bin haribote.sys

src_only :
	make clean
	rm haribote.img

clean_debug :
	rm bochsout.txt
