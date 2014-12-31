
void _io_hlt(void);
void _write_mem8(int addr, int data);
void io_cli(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);


void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
void init_screen8(char *vram, int xsize, int ysize);

void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);
void putfont8(char *vram, int xszie, int x, int y, char c, char *font);
void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s);
void init_mouse_cursor8(char *mouse, char bc);
void putblock8_8(char *vram, int vxsize, int pxsize,
		 int pysize, int px0, int py0, char *buf, int bxsize);

#define COL8_000000    0
#define COL8_FF0000    1
#define COL8_00FF00    2
#define COL8_FFFF00    3
#define COL8_0000FF    4
#define COL8_FF00FF    5
#define COL8_00FFFF    6
#define COL8_FFFFFF    7
#define COL8_C6C6C6    8
#define COL8_840000    9
#define COL8_008400    10
#define COL8_848400    11
#define COL8_000084    12
#define COL8_840084    13
#define COL8_008484    14
#define COL8_848484    15

struct BOOTINFO {
  char cyls, leds, vmode, reserve;
  short scrnx, scrny;
  char *vram;
};

struct SEGMENT_DESCRIPTOR{
  short limit_low, base_low;
  char base_mid, access_right;
  char limit_high, base_high;
};

struct GATE_DESCRIPTOR{
  short offset_low, selector;
  char dw_count, access_right;
  short offset_high;
};

void init_gdtidt(void);
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);

void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);

void HariMain(void)
{
  struct BOOTINFO *binfo = (struct BOOTINFO *) 0x0ff0;
  char s[40], mcursor[256];
  int mx, my;

  init_gdtidt();
  init_palette();
  init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);
  mx = (binfo->scrnx - 16) / 2;
  my = (binfo->scrny - 28 - 16) / 2;
  init_mouse_cursor8(mcursor, COL8_008484);
  putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
  // sprintf(s, "(%d, %d)", mx, my);
  //  putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);

   for(;;){
     _io_hlt();
  }

   return;
}

void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1)
{
   int x, y;
   for( y = y0; y <= y1; y++){
       for(x = x0; x <= x1; x++)
          vram[y * xsize + x] = c;
   }
   return;
}

void init_palette(void)
{/*如果是静态变量，分配在elf文件的数据段，因为暂时未写elf文件加载器，会导致地址指向不正确，暂时使用局部变量。此时局部变量占用空间有点大，编译器默认会使用C库的栈保护函数，编写此系统不使用C库，所以Makefile中，关闭此保护的链接：-fon-stack-protector*/
   unsigned char table_rgb[16 * 3] = {
    0x00, 0x00, 0x00, /* 0:黑 */
    0xff, 0x00, 0x00, /* 1:亮红 */
    0x00, 0xff, 0x00, /* 2:亮绿*/
    0xff, 0xff, 0x00, /* 3:亮黄*/
    0x00, 0x00, 0xff, /* 4:亮蓝*/
    0xff, 0x00, 0xff, /* 5:亮紫*/
    0x00, 0xff, 0xff, /* 6:浅亮蓝*/
    0xff, 0xff, 0xff, /* 7:白*/
    0xc6, 0xc6, 0xc6, /* 8:亮灰*/
    0x84, 0x00, 0x00, /* 9:暗红*/
    0x00, 0x84, 0x00, /*10:暗绿*/
    0x84, 0x84, 0x00, /*11:暗黄*/
    0x00, 0x00, 0x84, /*12:暗青*/
    0x84, 0x00, 0x84, /*13:暗紫*/
    0x00, 0x84, 0x84, /*14:浅暗蓝*/
    0x84, 0x84, 0x84  /*15:暗灰*/
  };
  set_palette(0, 15, table_rgb);
  return;
}

void set_palette(int start, int end, unsigned char *rgb)
{
  int i, eflags;
  eflags = io_load_eflags();
  io_cli();
  io_out8(0x03c8, start);
  for(i = start; i<=end; i++){
    io_out8(0x03c9, rgb[0] / 4);
    io_out8(0x03c9, rgb[1] / 4);
    io_out8(0x03c9, rgb[2] / 4);
    rgb += 3;
  }
  io_store_eflags(eflags);
  return;
}

void init_screen8(char *vram, int xsize, int ysize)
{
  boxfill8(vram, xsize, COL8_008484, 0, 0, xsize - 1, ysize - 29);
  boxfill8(vram, xsize, COL8_C6C6C6, 0, ysize - 28, xsize - 1, ysize - 28);
  boxfill8(vram, xsize, COL8_FFFFFF, 0, ysize - 27, xsize - 1, ysize - 27);
  boxfill8(vram, xsize, COL8_C6C6C6, 0, ysize - 26, xsize - 1, ysize - 1);

  boxfill8(vram, xsize, COL8_FFFFFF, 3, ysize - 24,        59, xsize - 24);/*最后一个参数由ysize-24错写为xsize-24*/
  boxfill8(vram, xsize, COL8_FFFFFF, 2, ysize - 24,         2, ysize - 4);
  boxfill8(vram, xsize, COL8_848484, 3, ysize -  4,        59, ysize - 4);
  boxfill8(vram, xsize, COL8_848484,59, ysize - 23,        59, ysize - 5);
  boxfill8(vram, xsize, COL8_000000, 2, ysize -  3,        59, ysize - 3);
  boxfill8(vram, xsize, COL8_000000,60, ysize - 24,        60, ysize - 3);

  boxfill8(vram, xsize, COL8_848484, xsize - 47, ysize - 24, xsize - 4, ysize -24);
  boxfill8(vram, xsize, COL8_848484, xsize - 47, ysize - 23, xsize - 47, ysize -4);
  boxfill8(vram, xsize, COL8_FFFFFF, xsize - 47, ysize - 23, xsize - 4, ysize - 3);/*第五个参数由ysize-3错写为ysize-23*/
  boxfill8(vram, xsize, COL8_FFFFFF, xsize - 3, ysize - 24, xsize - 3, ysize - 3);
  return;
}

void putfont8(char *vram, int xsize, int x, int y, char c, char *font)
{
  int i;
  char *p, d;

  for(i = 0; i < 16; i++)
    {
      p = vram + (y + i) * xsize + x;
      d = font[i];
      if(0 != (d & 0x80)) { p[0] = c;}
      if(0 != (d & 0x40)) { p[1] = c;}
      if(0 != (d & 0x20)) { p[2] = c;}
      if(0 != (d & 0x10)) { p[3] = c;}
      if(0 != (d & 0x08)) { p[4] = c;}
      if(0 != (d & 0x04)) { p[5] = c;}
      if(0 != (d & 0x02)) { p[6] = c;}
      if(0 != (d & 0x01)) { p[7] = c;}
    }
}

void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s)
{
  extern char _hankaku[4096];
  
  for(; *s!=0; ++s)
    {
      putfont8(vram, xsize, x, y, c, _hankaku + *s * 16);
      x += 8;
    }
}

void init_mouse_cursor8(char *mouse, char bc)
{
  static char cursor[16][16] = {
    "**************..",
    "*OOOOOOOOOOO*...",
    "*OOOOOOOOOO*....",
    "*OOOOOOOOO*.....",
    "*OOOOOOOO*......",
    "*OOOOOOO*.......",
    "*OOOOOOO*.......",
    "*OOOOOOOO*......",
    "*OOOO**OOO*.....",
    "*OOO*..*OOO*....",
    "*OO*....*OOO*...",
    "*O*......*OOO*..",
    "**........*OOO*.",
    "*..........*OOO*",
    "............*OO*",
    ".............***"
  };

  int x, y;
  
  for (y = 0; y < 16; y++) {
    for (x = 0; x < 16; x++) {
      if (cursor[y][x] == '*') {
	mouse[y * 16 + x] = COL8_000000;
      }
      if (cursor[y][x] == 'O') {
	mouse[y * 16 + x] = COL8_FFFFFF;
      }
      if (cursor[y][x] == '.') {
	mouse[y * 16 + x] = bc;
      }
    }
  }
  return;
}

void putblock8_8(char *vram, int vxsize, int pxsize,
		 int pysize, int px0, int py0, char *buf, int bxsize)
{
  int x, y;
  for (y = 0; y < pysize; y++) {
    for (x = 0; x < pxsize; x++) {
      vram[(py0 + y) * vxsize + (px0 + x)] = buf[y * bxsize + x];
    }
  }
  return;
}

void init_gdtidt(void)
{
  // 0x00270000-0x0027ffff GDT，2@13个项，每项2@3个字节，64K
  struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DECRIPTOR *) 0x00270000;
  // 0x0026f800-0x0026ffff IDT，2@8项，每项2@3个字节， 2K
  struct GATE_DESCRIPTOR *idt = (struct GATE_DESCRIPTOR *) 0x0026f800;
  int i;

  for(i = 0; i < 8192; i++) {
    set_segmdesc(gdt + i, 0, 0, 0);
  }
  set_segmdesc(gdt + 1, 0xffffffff, 0x00000000, 0x4092);
  set_segmdesc(gdt + 2, 0x0007ffff, 0x00280000, 0x409a);
  load_gdtr(0xffff, 0x00270000);

  for (i = 0; i < 256; i++) {
    set_gatedesc(idt + i, 0, 0, 0);
  }
  load_idtr(0x7ff, 0x0026f800);

  return;
}

void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar)
{
  if (limit > 0xfffff) {
    ar |= 0x8000;
    limit /= 0x1000;
  }

  sd->limit_low = limit & 0xffff;
  sd->base_low = base & 0xffff;
  sd->base_mid = (base >> 16) & 0xff;
  sd->access_right = ar & 0xff;
  sd->limit_high = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0);
  sd->base_high = (base >> 24) & 0xff;

  return;
}

void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar)
{
  gd->offset_low = offset & 0xffff;
  gd->selector = selector;
  gd->dw_count = (ar >> 8) & 0xff;
  gd->access_right = ar & 0xff;
  gd->offset_high = (offset >> 16) & 0xffff;
  return;
}
