#include "bootpack.h"

extern struct FIFO8 keyfifo;
extern struct FIFO8 mousefifo;
extern struct TIMERCTL timerctl;

#define MEMMAN_ADDR        0x003c0000

void make_window8(unsigned char *buf, int xsize, int ysize, char *title);
void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l);
void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c);
void task_b_main(struct SHEET *sht_back);

struct BOOTINFO *g_binfo;
char g_msg[64];

void HariMain(void)
{
  struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
  g_binfo = binfo;
	struct FIFO32 fifo;
  char s[40];
	int fifobuf[128]; 
	struct TIMER *timer, *timer2, *timer3;
  int mx, my, i;
	int cursor_x, cursor_c;
  unsigned int memtotal;
  struct MOUSE_DEC mdec;
  struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
  struct SHTCTL *shtctl;
  struct SHEET *sht_back, *sht_mouse, *sht_win;
  unsigned char *buf_back, buf_mouse[256], *buf_win;
	static char keytable[0x54] = {
		0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0, 0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0, 0, 'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0, 0, ']', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', ',', '.', '/', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.'
	};
	struct TSS32 tss_a, tss_b;
	int task_b_esp;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR*)ADR_GDT;
  init_gdtidt();
  init_pic();
  io_sti(); /* GDT,IDT,PIC*/
  fifo32_init(&fifo, 128, fifobuf);
	
	// timer
	init_pit();
	timer = timer_alloc();
	timer_init(timer, &fifo, 10);
	timer_settime(timer, 1000);
	timer2 = timer_alloc();
	timer_init(timer2, &fifo, 3);
	timer_settime(timer2, 300);
	timer3 = timer_alloc();
	timer_init(timer3, &fifo, 1);
	timer_settime(timer3, 50);	
	
  io_out8(PIC0_IMR, 0xf8); // 11111001,PIC1和IRQ1,
  io_out8(PIC1_IMR, 0xef); // 11101111, IRQ12
  init_keyboard(&fifo, 256);
  enable_mouse(&fifo, 512, &mdec);

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
	make_textbox8(sht_win, 8, 28, 144, 16, COL8_FFFFFF);
	cursor_x = 8;
	cursor_c = COL8_FFFFFF;
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

	tss_a.ldtr = 0;
	tss_a.iomap = 0x40000000;
	tss_b.ldtr = 0;
	tss_b.iomap = 0x40000000;
	set_segmdesc(gdt + 3, 103, (int) &tss_a, AR_TSS32);
	set_segmdesc(gdt + 4, 103, (int) &tss_b, AR_TSS32);
	load_tr(3 * 8);
	task_b_esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 8;
	*((int *) (task_b_esp + 4)) = (int) sht_back;
	tss_b.eip = (int)&task_b_main - ADR_BOTPAK;
	tss_b.eflags = 0x00000202; /* IF = 1*/
	tss_b.eax = 0;
	tss_b.ecx = 0;
	tss_b.edx = 0;
	tss_b.ebx = 0;
	tss_b.esp = task_b_esp;
	tss_b.ebp = 0;
	tss_b.esi = 0;
	tss_b.edi = 0;
	tss_b.es = 1 * 8;
	tss_b.cs = 2 * 8;
	tss_b.ss = 1 * 8;
	tss_b.ds = 1 * 8;
	tss_b.fs = 1 * 8;
	tss_b.gs = 1 * 8;

	mt_init();

  for(;;) {
     io_cli();
     if(0 == fifo32_status(&fifo)) {
	 	io_sti();
    }else{
		i = fifo32_get(&fifo);
		io_sti();
	 	if (256 <= i && i <= 511) {
     		sprintf(s, "0x%x", i - 256);
			putfonts8_asc_sht(sht_back, 0, 16, COL8_FFFFFF, COL8_008484, s, 8);
			if (i < 256 + 0x54) {
				if (0 != keytable[i - 256] && cursor_x < 144) {
					s[0] = keytable[i - 256];
					s[1] = 0;	
					putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_C6C6C6, s, 1);
					cursor_x += 8;
				}
			}
			if (i == 256 + 0x0e && cursor_x > 8) { // Backspace
				putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, " ", 1);
				cursor_x -= 8;	
			}
			boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
			sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
       	} // if key status 
		else if (512 <= i && i <= 767) {
	 		if (0 != mouse_decode(&mdec, i - 512)) {
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
				putfonts8_asc_sht(sht_back, 64, 16, COL8_FFFFFF, COL8_008484, s, 24);
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
				putfonts8_asc_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_008484, s, 16);
       	   		sheet_slide(sht_mouse, mx, my); /* 显示鼠标 */
				
				// 
				if ((mdec.btn & 0x01) != 0) {
					sheet_slide(sht_win, mx - 80, my - 8);	
				}
	 		} // if mouse decode
	  	} // if 512 767 
		else if (10 == i) {
			putfonts8_asc_sht(sht_back, 0, 64, COL8_FFFFFF, COL8_008484, "10[sec]", 7);
		} else if (3 == i) {
			putfonts8_asc_sht(sht_back, 0, 80, COL8_FFFFFF, COL8_008484, "3[sec]", 6);
		} else if (i <= 1) {
			if (0 != i) {
				timer_init(timer3, &fifo, 0); // set to 0
				cursor_c = COL8_000000;
			} else {
				timer_init(timer3, &fifo, 1);
				cursor_c = COL8_FFFFFF;
			}
			boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
			timer_settime(timer3, 50);
			sheet_refresh(sht_back, 8, 96, 16, 112);
		}
	} // else 0 == getstatus fifo 
  } // for

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

void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l)
{
	boxfill8(sht->buf, sht->bxsize, b, x, y, x + l * 8 - 1, y + 15);
	putfonts8_asc(sht->buf, sht->bxsize, x, y, c, s);
	sheet_refresh(sht, x, y, x + l * 8, y + 16);
	return;
}

void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c)
{
	int x1 = x0 + sx, y1 = y0 + sy;
	boxfill8(sht->buf, sht->bxsize, COL8_848484, x0 - 2, y0 - 3, x1 + 1, y0 - 3);
	boxfill8(sht->buf, sht->bxsize, COL8_848484, x0 - 3, y0 - 3, x0 - 3, y1 + 1);
	boxfill8(sht->buf, sht->bxsize, COL8_FFFFFF, x1 - 3, y1 + 2, x1 + 1, y1 + 2);
	boxfill8(sht->buf, sht->bxsize, COL8_FFFFFF, x1 + 2, y0 - 3, x1 + 2, y1 + 2);
	boxfill8(sht->buf, sht->bxsize, COL8_000000, x0 - 1, y0 - 2, x1 + 0, y0 - 2);
	boxfill8(sht->buf, sht->bxsize, COL8_000000, x0 - 2, y0 - 2, x0 - 2, y1 + 0);
	boxfill8(sht->buf, sht->bxsize, COL8_C6C6C6, x0 - 2, y1 + 1, x1 + 0, y1 + 1);
	boxfill8(sht->buf, sht->bxsize, COL8_C6C6C6, x1 + 1, y0 - 2, x1 + 1, y1 + 1);
	boxfill8(sht->buf, sht->bxsize, c,			 x0 - 1, y0 - 1, x1 + 0, y1 + 0);
	return;
}

void task_b_main(struct SHEET *sht_back)
{
	struct FIFO32 fifo;
	struct TIMER *timer_ls, *timer_put;
	int i, fifobuf[128], count = 0, count0 = 0;
	char s[11];

	fifo32_init(&fifo, 128, fifobuf);
	timer_ls = timer_alloc();
	timer_init(timer_ls, &fifo, 100);
	timer_settime(timer_ls, 100);
	timer_put = timer_alloc();
	timer_init(timer_put, &fifo, 1);
//	timer_settime(timer_put, 1);

	for (;;) {
		count++;
		io_cli();
		if (0 == fifo32_status(&fifo)) {
			io_sti();
		} else {
			i = fifo32_get(&fifo);
			io_sti();
			if (1 == i) {
				sprintf(s, "%x", count);
				putfonts8_asc_sht(sht_back, 0, 144, COL8_FFFFFF, COL8_008484, s, 10);
				timer_settime(timer_put, 1);
			} else if (100 == i) {
				sprintf(s, "%x", count - count0);
				putfonts8_asc_sht(sht_back, 0, 128, COL8_FFFFFF, COL8_008484, s, 11);
				count0 = count;
				timer_settime(timer_ls, 100);
			}
		}
	}
}
