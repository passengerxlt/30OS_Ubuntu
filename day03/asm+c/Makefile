#
default:
	make img_i386

foo_i386.o : foo.asm Makefile
	nasm -f elf foo.asm -o foo_i386.o

bar_i386.o : bar.c Makefile
	gcc -m32 -c bar.c -o bar_i386.o 

foobar_i386 : foo_i386.o bar_i386.o Makefile
	ld -m elf_i386  -s -o foobar_i386 foo_i386.o bar_i386.o

img_i386:
	make -r foobar_i386

run:
	make img_i386
	./foobar_i386

clean:
	rm foo_i386.o bar_i386.o

src_only:
	make clean
	rm foobar_i386
