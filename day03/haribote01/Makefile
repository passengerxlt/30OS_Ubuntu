#默认命令
default:
	make img

#文件生成规则

ipl.bin : ipl.asm Makefile
	nasm ipl.asm -o ipl.bin -l ipl.lst

ciwawa.img : ipl.bin Makefile
	qemu-img create ciwawa.img 1440k
	dd if=ipl.bin of=ciwawa.img bs=512 count=1 conv=notrunc

#命令

img :
	make -r ciwawa.img

asm :
	make -r ipl.bin

run :
	make img
	qemu-system-i386 -fda ciwawa.img -boot a -no-acpi

#安装到软盘上
install :

clean :
	rm ipl.bin ipl.lst

src_only :
	make clean
	rm ciwawa.img
