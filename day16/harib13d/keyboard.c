#include "bootpack.h"

struct FIFO32 *keyfifo;
int keydata0;

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

void init_keyboard(struct FIFO32 *fifo, int data0)
{
	keyfifo = fifo;
	keydata0 = data0;

  /* 初始化键盘控制电路 */
  wait_KBC_sendready();
  io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
  wait_KBC_sendready();
  io_out8(PORT_KEYDAT, KBC_MODE);
  return;
}

void inthandler21(int *esp)
{
 	int data;
  data = io_in8(PORT_KEYDAT);
  io_out8(PIC0_OCW2, 0x61); /* 通知PIC“IRQ-01已经受理完毕”*/
  fifo32_put(keyfifo, data + keydata0);
  return;
}
