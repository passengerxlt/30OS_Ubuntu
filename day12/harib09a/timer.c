#define PIT_CTRL    0x0043
#define PIT_CNT0    0x0040

#include "bootpack.h"

struct TIMERCTL timerctl;

void init_pit(void)
{
  io_out8(PIT_CTRL, 0x34);
  io_out8(PIT_CNT0, 0x9e);
  io_out8(PIT_CNT0, 0x2e);
	timerctl.count = 0;
  return;
}

void inthandler20(int *esp)
{
  io_out8(PIC0_OCW2, 0x60);
	timerctl.count++;
  return;
}