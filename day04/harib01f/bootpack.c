void _io_hlt(void);
void _write_mem8(int addr, int data);
void io_cli(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);


void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);

void HariMain(void)
{
  int i;

  for(i = 0xa0000; i <= 0xaffff; i++) {
    *((char *) i) = i & 0x0f;
  }

  init_palette();

  for(i = 0xa0000; i <= 0xaffff; i++) {
    *((char *) i) = i & 0x0f;
  }

  for(;;){
    _io_hlt();
  }
}

void init_palette(void)
{ /*如果是静态变量，分配在elf文件的数据段，因为暂时未写elf文件加载器，会导致地址指向不正确，暂时使用局部变量。此时局部变量占用空间有点大，编译器默认会使用C库的栈保护函数，编写此系统不使用C库，所以Makefile中，关闭此保护的链接：-fon-stack-protector*/
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
  for(i = start; i <= end; i++){
    io_out8(0x03c9, rgb[0] / 4);
    io_out8(0x03c9, rgb[1] / 4);
    io_out8(0x03c9, rgb[2] / 4);
    rgb += 3;
  }
  io_store_eflags(eflags);
  return;
}
