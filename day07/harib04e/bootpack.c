#include "bootpack.h"

extern struct FIFO8 keyfifo;

void HariMain(void)
{
  struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
  char s[40], mcursor[256], keybuf[32];
  int mx, my;

  init_gdtidt();
  init_pic();
  io_sti();

  init_palette();
  init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);
  mx = (binfo->scrnx - 16) / 2;
  my = (binfo->scrny - 28 - 16) / 2;
  init_mouse_cursor8(mcursor, COL8_008484);
  putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
  sprintf(s, "(%x, %x)", mx, my);
  putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);

  fifo8_init(&keyfifo, 32, keybuf);

  io_out8(PIC0_IMR, 0xf9); // 11111001,PIC1和IRQ1,
  //  io_out8(PIC0_IMR, 0xfb); // 11111011,PIC1
  io_out8(PIC1_IMR, 0xef); // 11101111, IRQ12

  //  testio2c();

   for(;;){
     io_cli();
     if(0 == fifo8_status(&keyfifo)) {
	 io_stihlt();
       } else {
	int i = fifo8_get(&keyfifo);
	 io_sti();
	 sprintf(s, "%x", i);
	 boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 16, 15, 31);
	 putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
       }
  }

   return;
}


