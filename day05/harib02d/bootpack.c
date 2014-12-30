
void _io_hlt(void);
void _write_mem8(int addr, int data);
void io_cli(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);


void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
void init_screen(char *vram, int xsize, int ysize);

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

void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);

static char font_A[16] = {
  0x00, 0x18, 0x18, 0x18, 0x18, 0x24, 0x24, 0x24,
  0x24, 0x7e, 0x42, 0x42, 0x42, 0xe7, 0x00, 0x00
};

void putfont8(char *vram, int xszie, int x, int y, char c, char *font);

void HariMain(void)
{
  char *vram;
  int xsize, ysize;
  short *binfo_scrnx, *binfo_scrny;
  int *binfo_vram;

  init_palette();
  binfo_scrnx = (short*) 0x0ff4;
  binfo_scrny = (short*) 0x0ff6;
  binfo_vram = (int*) 0x0ff8;
  xsize = *binfo_scrnx;
  ysize = *binfo_scrny;
  vram = (char*)*binfo_vram;

  init_screen(vram, xsize, ysize);

  putfont8(vram, xsize, 10, 10, COL8_C6C6C6, font_A);

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

void init_screen(char *vram, int xsize, int ysize)
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
