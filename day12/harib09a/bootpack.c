#include "bootpack.h"

extern struct FIFO8 keyfifo;
extern struct FIFO8 mousefifo;
extern struct TIMERCTL timerctl;

#define MEMMAN_ADDR        0x003c0000

void make_window8(unsigned char *buf, int xsize, int ysize, char *title);

struct BOOTINFO *g_binfo;

void HariMain(void)
{
  struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
  g_binfo = binfo;
  char s[40], keybuf[32], mousebuf[128];
  int mx, my, i;
  unsigned int memtotal, count = 0;
  struct MOUSE_DEC mdec;
  struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
  struct SHTCTL *shtctl;
  struct SHEET *sht_back, *sht_mouse, *sht_win;
  unsigned char *buf_back, buf_mouse[256], *buf_win;

  init_gdtidt();
  init_pic();
  io_sti(); /* GDT,IDT,PIC*/
  fifo8_init(&keyfifo, 32, keybuf);
  fifo8_init(&mousefifo, 128, mousebuf);
	init_pit();

  io_out8(PIC0_IMR, 0xf8); // 11111001,PIC1和IRQ1,
  io_out8(PIC1_IMR, 0xef); // 11101111, IRQ12
  init_keyboard();
  enable_mouse(&mdec);

  memtotal = memtest(0x00400000, 0xbfffffff);
  memman_init(memman);
  memman_free(memman, 0x00001000, 0x0009e000);
  memman_free(memman, 0x00400000, memtotal - 0x00400000);

  init_palette();
  shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
  sht_back = sheet_alloc(shtctl);
  sht_mouse = sheet_alloc(shtctl);
  sht_win = sheet_alloc(shtctl);
  buf_back = (unsigned char *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
  buf_win = (unsigned char *) memman_alloc_4k(memman, 160 * 52);

  sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1);
  sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
  sheet_setbuf(sht_win, buf_win, 160, 52, -1); /*没有透明色*/
  init_screen8(buf_back, binfo->scrnx, binfo->scrny);

  init_mouse_cursor8(buf_mouse, 99);
  make_window8(buf_win, 160, 52, "counter");
  sheet_slide(sht_back, 0, 0);
  mx = (binfo->scrnx - 16) / 2;
  my = (binfo->scrny - 28 - 16) / 2;
  sheet_slide(sht_mouse, mx, my);
  sheet_slide(sht_win, 80, 72);
  
  sheet_updown(sht_back, 0);
  sheet_updown(sht_win, 1);
  sheet_updown(sht_mouse, 2);

  sprintf(s, "(0x%x, 0x%x)", mx, my);
  putfonts8_asc(buf_back, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
  sprintf(s, "memory 0x%xMB, free : 0x%xKB", memtotal / (1024 * 1024), memman_total(memman) / 1024);
  putfonts8_asc(buf_back, binfo->scrnx, 0, 32, COL8_FFFFFF, s);
  sheet_refresh(sht_back, 0, 0, binfo->scrnx, 48);

  for(;;) {
    count++;
    sprintf(s, "0x%x", timerctl.count);
    boxfill8(buf_win, 160, COL8_C6C6C6, 40, 28, 119, 43);
    putfonts8_asc(buf_win, 160, 40, 28, COL8_000000, s);

    sheet_refresh(sht_win, 40, 28, 120, 44);

     io_cli();
     if(0 == fifo8_status(&keyfifo) + fifo8_status(&mousefifo)) {
	 io_sti();
     } else {
       if(0 != fifo8_status(&keyfifo)) {
	 i = fifo8_get(&keyfifo);
	 io_sti();
     	 sprintf(s, "0x%x", i);
       	 boxfill8(buf_back, binfo->scrnx, COL8_008484, 0, 16, 8 * 8 - 1, 31);
       	 putfonts8_asc(buf_back, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
       	 sheet_refresh(sht_back, 0, 16, 8 * 8, 32);
       } else if (0 != fifo8_status(&mousefifo)) {
	 i = fifo8_get(&mousefifo);
	 io_sti();
	 if (0 != mouse_decode(&mdec, i)) {
	   sprintf(s, "[lcr 0x%x 0x%x 0x%x]", mdec.buf[0], mdec.buf[1], mdec.buf[2]);
	   if (0 != (mdec.btn & 0x01)) {
	     s[1] = 'L';
	   }
	   if (0 != (mdec.btn & 0x02)) {
	     s[3] = 'R';
	   }
	   if (0 != (mdec.btn & 0x04)) {
	     s[2] = 'C';
	   }
       	   boxfill8(buf_back, binfo->scrnx, COL8_008484, 64, 16, 64 + 24 * 8 - 1, 31);
       	   putfonts8_asc(buf_back, binfo->scrnx, 64, 16, COL8_FFFFFF, s);
       	   sheet_refresh(sht_back, 64, 16, 64 + 24 * 8, 32);
	   mx += mdec.x;
	   my += mdec.y;
	   if (mx < 0) {
	     mx = 0;
	   }
	   if (my < 0) {
	     my = 0;
	   }
	   if (mx > binfo->scrnx - 1) {
	     mx = binfo->scrnx - 1;
	   }
	   if (my > binfo->scrny - 1) {
	     my = binfo->scrny - 1;
	   }
	   sprintf(s, "(0x%x, 0x%x)", mx, my);
       	   boxfill8(buf_back, binfo->scrnx, COL8_008484, 0, 0, 8 * 16 - 1, 15); /* 隐藏坐标*/
       	   putfonts8_asc(buf_back, binfo->scrnx, 0, 0, COL8_FFFFFF, s); /* 显示坐标*/
       	   sheet_refresh(sht_back, 0, 0, 8 * 16, 16);
       	   sheet_slide(sht_mouse, mx, my); /* 显示鼠标 */
	 }
       }
     }
   }

   return;
}

void make_window8(unsigned char *buf, int xsize, int ysize, char *title)
{
  static char closebtn[14][16] = {
    "OOOOOOOOOOOOOOO@",
    "OQQQQQQQQQQQQQ$@",
    "OQQQQQQQQQQQQQ$@",
    "OQQQ@@QQQQ@@QQ$@",
    "OQQQQ@@QQ@@QQQ$@",
    "OQQQQQ@@@@QQQQ$@",
    "OQQQQQQ@@QQQQQ$@",
    "OQQQQQ@@@@QQQQ$@",
    "OQQQQ@@QQ@@QQQ$@",
    "OQQQ@@QQQQ@@QQ$@",
    "OQQQQQQQQQQQQQ$@",
    "OQQQQQQQQQQQQQ$@",
    "O$$$$$$$$$$$$$$@",
    "@@@@@@@@@@@@@@@@"
  };
  int x, y;
  char c;
  boxfill8(buf, xsize, COL8_C6C6C6, 0, 0, xsize - 1, 0); /*上边-第一条线*/
  boxfill8(buf, xsize, COL8_FFFFFF, 1, 1, xsize - 2, 1); /*上边第二条线*/
  boxfill8(buf, xsize, COL8_C6C6C6, 0, 0, 0, ysize - 1); /*左边第一条线*/
  boxfill8(buf, xsize, COL8_FFFFFF, 1, 1, 1, ysize - 2); /*左边第二条线*/
  boxfill8(buf, xsize, COL8_848484, xsize - 2, 1, xsize - 2, ysize - 2); /*右边第二条线*/
  boxfill8(buf, xsize, COL8_000000, xsize - 1, 0, xsize - 1, ysize - 1); /*右边第一条线*/
  boxfill8(buf, xsize, COL8_C6C6C6, 2, 2, xsize - 3, ysize - 3); /*背景*/
  boxfill8(buf, xsize, COL8_000084, 3, 3, xsize - 4, 20); /*标题栏*/
  boxfill8(buf, xsize, COL8_848484, 1, ysize - 2, xsize - 2, ysize - 2); /*下面第二条线*/
  boxfill8(buf, xsize, COL8_000000, 0, ysize - 1, xsize - 1, ysize - 1); /*下面地一条线*/
  putfonts8_asc(buf, xsize, 24, 4, COL8_FFFFFF, title);/*标题*/
  for (y = 0; y < 14; y++) {
    for (x = 0; x < 16; x++) {
      c = closebtn[y][x];
      if ('@' == c) {
	c = COL8_000000;
      } else if ('$' == c) {
	c = COL8_848484;
      } else if ('Q' == c) {
 	c = COL8_C6C6C6;
      } else {
	c = COL8_FFFFFF;
      }
      buf[(5 + y) * xsize + (xsize - 21 + x)] = c; /*关闭按钮*/
    }
  }
  return;
}
