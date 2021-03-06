#include "bootpack.h"

extern struct FIFO8 keyfifo;
extern struct FIFO8 mousefifo;
struct MOUSE_DEC {
  unsigned char buf[3], phase;
  int x, y, btn;
};

void enable_mouse(struct MOUSE_DEC *mdec);
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat);
void init_keyboard(void);

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

#define PORT_KEYDAT          0x0060
#define PORT_KEYSTA          0x0064
#define PORT_KEYCMD          0x0064
#define KEYSTA_SEND_NOTREADY 0x02
#define KEYCMD_WRITE_MODE    0x60
#define KBC_MODE             0x47

void wait_KBC_sendready(void)
{
  /* 等待键盘控制电路准备完毕 */
  for (;;) {
    if (0 == (KEYSTA_SEND_NOTREADY & io_in8(PORT_KEYSTA))) {
	break;
      }
  }
    return;
}

void init_keyboard(void)
{
  /* 初始化键盘控制电路 */
  wait_KBC_sendready();
  io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
  wait_KBC_sendready();
  io_out8(PORT_KEYDAT, KBC_MODE);
  return;
}

#define KEYCMD_SENDTO_MOUSE  0xd4
#define MOUSECMD_ENABLE      0xf4

void enable_mouse(struct MOUSE_DEC *mdec)
{
  /* 激活鼠标 */
  wait_KBC_sendready();
  io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
  wait_KBC_sendready();
  io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
  mdec->phase = 0;
  return; /* 顺利的话，键盘控制器会返送回ACK（0xfa） */
}

int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat)
{
  if (0 == mdec->phase) {
    if (0xfa == dat) {
      mdec->phase = 1;
    }
    return 0;
  }
  if (1 == mdec->phase) {
    /*等待鼠标第一字节*/
    if ( 0x08 == (dat & 0xc8)) {
      /* 检查第一个字节是否正确，不正确，后续的舍去*/
      mdec->buf[0] = dat;
      mdec->phase = 2;
    }
    return 0;
  }
  if (2 == mdec->phase) {
    /* 等待鼠标第二字节*/
    mdec->buf[1] = dat;
    mdec->phase = 3;
    return 0;
  }
  if (3 == mdec->phase) {
    /*等待鼠标第三字节*/
    mdec->buf[2] = dat;
    mdec->phase = 1;
    mdec->btn = mdec->buf[0] & 0x07; // 低3位
    mdec->x = mdec->buf[1];
    mdec->y = mdec->buf[2];
    
    if (0 != (mdec->buf[0] & 0x10)) {
      mdec->x |= 0xffffff00;
    }
    if (0 != (mdec->buf[0] & 0x20)) {
      mdec->y |= 0xffffff00;
    }
    mdec->y = - mdec->y; /*鼠标的y方向与画面符号相反*/
    return 1;
  }
  return -1;
}
