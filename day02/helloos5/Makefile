#默认命令
default:
	make img

#文件生成规则

ipl.bin : ipl.asm Makefile
	nasm ipl.asm -o ipl.bin -l ipl.lst

boot.img : ipl.bin Makefile
	qemu-img create boot.img 1440k
	dd if=ipl.bin of=boot.img bs=512 count=1 conv=notrunc

#命令

img :
	make -r boot.img

asm :
	make -r ipl.bin

run :
	make img
	qemu-system-i386 -fda boot.img -boot a -no-acpi

debug :
	bochs -q

#安装到软盘上
install :

clean :
	rm ipl.bin ipl.lst

src_only :
	make clean
	rm boot.img
	rm bochsout.txt
