#include "bootpack.h"

void sheet_refresh(struct SHTCTL *ctl);

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
  sht->col_inv;
  return;
}

void sheet_updown(struct SHTCTL *ctl, struct SHEET *sht, int height)
{
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
    sheet_refresh(ctl);
  }
  return;
}

void sheet_refresh(struct SHTCTL *ctl)
{
  int h, bx, by, vx, vy;
  unsigned char *buf, c, *vram = ctl->vram;
  struct SHEET *sht;
  unsigned char s[16];
  for (h = 0; h <= ctl->top; h++) {
    sht = ctl->sheets[h];
    buf = sht->buf;
    for (by = 0; by < sht->bysize; by++) {
      vy = sht->vy0 + by;
      for (bx = 0; bx < sht->bxsize; bx++) {
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

void sheet_slide(struct SHTCTL *ctl, struct SHEET *sht, int vx0, int vy0)
{
  sht->vx0 = vx0;
  sht->vy0 = vy0;
  if (0 <= sht->height) {
    sheet_refresh(ctl);
  }
}

void sheet_free(struct SHTCTL *ctl, struct SHEET *sht)
{
  if (0 <= sht->height) {
    sheet_updown(ctl, sht, -1);
  }
  sht->flags = 0;
  return;
}
