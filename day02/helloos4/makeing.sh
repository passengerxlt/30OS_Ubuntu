qemu-img create boot.img 1440k
dd if=ipl.bin of=boot.img bs=512 count=1 conv=notrunc
