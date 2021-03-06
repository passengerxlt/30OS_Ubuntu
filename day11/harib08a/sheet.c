#include "bootpack.h"

struct SHTCTL * shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize)
{
  struct SHTCTL *ctl;
  int i;
  ctl = (struct SHTCTL *) memman_alloc_4k(memman, sizeof(struct SHTCTL));
  if (0 == ctl) {
    goto err;
  }
  ctl->vram = vram;
  ctl->xsize = xsize;
  ctl->ysize = ysize;
  ctl->top = -1; /* 一个SHEET都没有*/
  for (i = 0; i < MAX_SHEETS; i++) {
    ctl->sheets0[i].flags = 0; /* 标记为未使用*/
    ctl->sheets0[i].ctl = ctl;
  }
 err:
  return ctl;
}

#define SHEET_USE     1

struct SHEET * sheet_alloc(struct SHTCTL *ctl)
{
  struct SHEET *sht;
  int i;
  for (i = 0; i < MAX_SHEETS; i++) {
    if (0 == ctl->sheets0[i].flags) {
      sht = &ctl->sheets0[i];
      sht->flags = SHEET_USE; /* 标记为正在使用*/
      sht->height = -1; /* 隐藏*/
      return sht;
    }
  }
  return 0;
}

void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int bxsize, int bysize, int col_inv)
{
  sht->buf = buf;
  sht->bxsize = bxsize;
  sht->bysize = bysize;
  sht->col_inv = col_inv;
  return;
}

void sheet_updown(struct SHEET *sht, int height)
{
  struct SHTCTL *ctl = sht->ctl;
  int h, old = sht->height; /*存储设置前的高度信息*/
  
  if (height > ctl->top + 1) {
    height = ctl->top + 1;
  }
  if (height < -1) {
    height = -1;
  }
  sht->height = height; /* 设置高度*/

  /*sheets[]按所指的SHEET的height从低到高排序。如果高度发生变化，重新排序，否则什么也不做*/
  if (old > height) { /*比以前低*/
    if (height >= 0) {
      /*把中间的往上拉（height的顺序），即在数组中向后移动一位*/
      for (h = old; h < height; h++) {
	ctl->sheets[h] = ctl->sheets[h - 1];
	ctl->sheets[h]->height = h;
      }
      ctl->sheets[height] = sht;
    } else {
      /*由显示-》隐藏，后面的高度都降一个，即在数组中向前移动一位*/
      if (ctl->top > old) {
	for (h = old; h < ctl->top; h++) {
	  ctl->sheets[h] = ctl->sheets[h + 1];
	  ctl->sheets[h]->height = h;
	}
      }
      ctl->top--;
    }
  } else if (old < height) { /*比以前高*/
    if (old >= 0) {
      /*把中间的拉下来*/
      for (h = old; h < height; h++) {
	ctl->sheets[h] = ctl->sheets[h + 1];
	ctl->sheets[h]->height = h;
      }
      ctl->sheets[height] = sht;
    } else {
      /*隐藏转为显示，中间插入一个，上面的再提上一次*/
      for (h = ctl->top; h >= height; h--) {
	ctl->sheets[h + 1] = ctl->sheets[h];
	ctl->sheets[h + 1]->height = h + 1;
      }
      ctl->sheets[height] = sht;
      ctl->top++;
    }
  }

  if (old != height) {
    sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht-> bysize);
  }
  return;
}

void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1)
{
  if (sht->height >= 0) {
    sheet_refreshsub(sht->ctl, sht->vx0 + bx0, sht->vy0 + by0, sht->vx0 + bx1, sht->vy0 + by1);
  }
  return;
}

void sheet_slide(struct SHEET *sht, int vx0, int vy0)
{
  int vx0_old = sht->vx0, vy0_old = sht->vy0;
  sht->vx0 = vx0;
  sht->vy0 = vy0;
  if (0 <= sht->height) {
    sheet_refreshsub(sht->ctl, vx0_old, vy0_old, vx0_old + sht->bxsize, vy0_old + sht->bysize);
    sheet_refreshsub(sht->ctl, vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize);
  }
}

void sheet_free(struct SHEET *sht)
{
  if (0 <= sht->height) {
    sheet_updown(sht, -1);
  }
  sht->flags = 0;
  return;
}

void sheet_refreshsub(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1)
{
  int h, bx, by, vx, vy, bx0, by0, bx1, by1;
  unsigned char *buf, c, *vram = ctl->vram;
  struct SHEET *sht;
  /* 画面之外的不做刷新*/
  if (vx0 < 0) { vx0 = 0; }
  if (vy0 < 0) { vy0 = 0; }
  if (vx1 > ctl->xsize) { vx1 = ctl->xsize; }
  if (vy1 > ctl->ysize) { vy1 = ctl->ysize; }
  for (h = 0; h <= ctl->top; h++) {
    sht = ctl->sheets[h];
    buf = sht->buf;
    /*在bx0-bx1，by0-by1之间循环，不再在0-bysize，0-bxsize之间循环*/
    bx0 = vx0 - sht->vx0;
    by0 = vy0 - sht->vy0;
    bx1 = vx1 - sht->vx0;
    by1 = vy1 - sht->vy0;
    if (bx0 < 0) { bx0 = 0; }
    if (by0 < 0) { by0 = 0; }
    if (bx1 > sht->bxsize) { bx1 = sht->bxsize; }
    if (by1 > sht->bysize) { by1 = sht->bysize; }
    for (by = by0; by < by1; by++) {
      vy = sht->vy0 + by;
      for (bx = bx0; bx < bx1; bx++) {
	vx = sht->vx0 + bx;
	c = buf[by * sht->bxsize + bx];
	if (c != sht->col_inv) {
	   vram[vy * ctl->xsize + vx] = c; // 图层在显存中对应的存储单元写入buf中对应的色号
	}
      }
    }
  }
  return;
}
