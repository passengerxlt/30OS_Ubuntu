#默认命令
default:
	make img

#文件生成规则

ipl.bin : ipl.asm Makefile
	nasm ipl.asm -o ipl.bin -l ipl.lst

haribote.sys : haribote.asm Makefile
	nasm haribote.asm -o haribote.sys -l haribote.lst

haribote.img : ipl.bin haribote.sys Makefile
#	qemu-img create haribote.img 1440k
	dd if=/dev/zero of=haribote.img bs=1024 count=1440
	dd if=ipl.bin of=haribote.img bs=512 count=1 conv=notrunc
	sudo mount haribote.img /mnt/floppy/ -o loop
	sudo cp haribote.sys /mnt/floppy
	sudo umount /mnt/floppy

#命令

img :
	make -r haribote.img

asm :
	make -r ipl.bin
	make -r haribote.sys

run :
	make img
	qemu-system-i386 -fda haribote.img -boot a -no-acpi

debug :
	bochs -q

#安装到软盘上
install :

clean :
	rm ipl.bin ipl.lst haribote.sys haribote.lst

src_only :
	make clean
	rm haribote.img

clean_debug :
	rm bochsout.txt
