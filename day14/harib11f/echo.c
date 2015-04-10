#include "bootpack.h"

void echo(struct BOOTINFO *binfo, unsigned char *s)
{
    boxfill8(binfo->vram, binfo->scrnx, COL8_C6C6C6, 0, 0, binfo->scrnx - 1, 16);
  putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
}
