#include "bootpack.h"

void echo(struct BOOTINFO *binfo, unsigned char *s)
{
  putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
}
