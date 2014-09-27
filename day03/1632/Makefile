default:
	make img

ipl10.bin : ipl10.asm Makefile
	nasm ipl10.asm -o ipl10.bin -l ipl10.lst

haribote.img : ipl10.bin Makefile
	dd if=/dev/zero	of=haribote.img bs=1024 count=1440
	dd if=ipl10.bin of=haribote.img bs=512 count=1 conv=notrunc
#
img :
	make -r haribote.img

asm :
	make -r ipl10.bin

run :
	make img
	qemu-system-i386 -fda haribote.img -boot a -no-acpi

debug :
	bochs -q

umount :
	sudo umount /mnt/floppy

clean :
	rm ipl10.bin ipl10.lst

src_only :
	make clean
	rm haribote.img bochsout.txt

clean_debug :
	rm bochsout.txt
