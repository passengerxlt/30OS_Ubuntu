#include "bootpack.h"

struct FIFO32 *mousefifo;
int mousedata0;

void enable_mouse(struct FIFO32 *fifo, int data0, struct MOUSE_DEC *mdec)
{
	mousefifo = fifo;
	mousedata0 = data0;

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

void inthandler2c(int *esp)
{
	int data;
  data = io_in8(PORT_KEYDAT);
  io_out8(PIC1_OCW2, 0x64); /* 通知PIC1 第4个即IRQ12已处理完成*/
  io_out8(PIC0_OCW2, 0x62); /* 通知PIC0 IRQ02已处理完成*/
  fifo32_put(mousefifo, data + mousedata0);
  return;
}
