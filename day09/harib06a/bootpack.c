#include "bootpack.h"

extern struct FIFO8 keyfifo;
extern struct FIFO8 mousefifo;

void HariMain(void)
{
  struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
  char s[40], mcursor[256], keybuf[32], mousebuf[128];
  int mx, my, i;
  struct MOUSE_DEC mdec;

  init_gdtidt();
  init_pic();
  io_sti();
  fifo8_init(&keyfifo, 32, keybuf);
  fifo8_init(&mousefifo, 128, mousebuf);
  io_out8(PIC0_IMR, 0xf9); // 11111001,PIC1和IRQ1,
  io_out8(PIC1_IMR, 0xef); // 11101111, IRQ12

  init_keyboard();

  init_palette();
  init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);
  mx = (binfo->scrnx - 16) / 2;
  my = (binfo->scrny - 28 - 16) / 2;
  init_mouse_cursor8(mcursor, COL8_008484);
  putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
  sprintf(s, "(%x, %x)", mx, my);
  putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);

  enable_mouse(&mdec);
  
   for(;;) {
     io_cli();
     if(0 == fifo8_status(&keyfifo) + fifo8_status(&mousefifo)) {
	 io_stihlt();
     } else {
       if(0 != fifo8_status(&keyfifo)) {
	 i = fifo8_get(&keyfifo);
	 io_sti();
	 sprintf(s, "%x", i);
	 boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 16, 15, 31);
	 putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
       } else if (0 != fifo8_status(&mousefifo)) {
	 i = fifo8_get(&mousefifo);
	 io_sti();
	 if (0 != mouse_decode(&mdec, i)) {
	   sprintf(s, "[lcr %x %x %x]", mdec.buf[0], mdec.buf[1], mdec.buf[2]);
	   if (0 != (mdec.btn & 0x01)) {
	     s[1] = 'L';
	   }
	   if (0 != (mdec.btn & 0x02)) {
	     s[3] = 'R';
	   }
	   if (0 != (mdec.btn & 0x04)) {
	     s[2] = 'C';
	   }
	   boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 32, 16, 32 + 15 * 8 - 1, 31);
	   putfonts8_asc(binfo->vram, binfo->scrnx, 32, 16, COL8_FFFFFF, s);
	   /* 鼠标指针移动 */
	   boxfill8(binfo->vram, binfo->scrnx, COL8_008484, mx, my, mx + 15, my + 15); /* 隐藏鼠标*/
	   mx += mdec.x;
	   my += mdec.y;
	   if (mx < 0) {
	     mx = 0;
	   }
	   if (my < 0) {
	     my = 0;
	   }
	   if (mx > binfo->scrnx - 16) {
	     mx = binfo->scrnx - 16;
	   }
	   if (my > binfo->scrny - 16) {
	     my = binfo->scrny - 16;
	   }
	   sprintf(s, "(%x, %x)", mx, my);
	   boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 0, 79, 15); /* 隐藏坐标*/
	   putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s); /* 显示坐标*/
	   putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16); /* 显示鼠标 */
	 }
       }
     }
   }

   return;
}
