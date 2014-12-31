/**/

#include "bootpack.h"

void init_pic(void)
/*PIC initial*/
{
  io_out8(PIC0_IMR, 0xff); // 8个都屏蔽
  io_out8(PIC1_IMR, 0xff); // 

  // ICW1和ICW4与主板配线方式、中断信号的电气特性有关
  // ICW3是有关主-从连接的设定
  io_out8(PIC0_ICW1, 0x11); // 边沿触发模式（edge trigger mode）
  io_out8(PIC0_ICW2, 0x20); // IRQ0-7由INT20-27接收
  io_out8(PIC0_ICW3, 1 << 2); // PIC1由IRQ2连接
  io_out8(PIC0_ICW4, 0x01); // 无缓冲区模式

  io_out8(PIC1_ICW1, 0x11);
  io_out8(PIC1_ICW2, 0x28); // IRQ8-IRQ15由INT18-2f接收
  io_out8(PIC1_ICW3, 2);   // PIC1由IRQ2链接
  io_out8(PIC1_ICW4, 0x01);

  io_out8(PIC0_IMR, 0xfb); // 11111011 PIC1以外全部禁止
  io_out8(PIC1_IMR, 0xff); // 11111111 禁止所有中断

  return;
}

void inthandler21(int *esp)
{
  struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
  boxfill8(binfo->vram, binfo->scrnx, COL8_000000, 0, 0, 32* 8 - 1, 15);
  putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, "INT 21 (IRQ-1) : PS/2 keyboard");
  for (;;) {
    _io_hlt();
  }
}

void inthandler27(int *esp)
{
  io_out8(PIC0_OCW2, 0x67);
  return;
}

void inthandler2c(int *esp)
{
  struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
  boxfill8(binfo->vram, binfo->scrnx, COL8_000000, 0, 0, 32* 8 - 1, 15);
  putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, "INT 21 (IRQ-1) : PS/2 mouse");
  for (;;) {
    _io_hlt();
  }
}

void testio21()
{
  io_int21();
  return;
}

void testio2c()
{
  io_int2c();
  return;
}
