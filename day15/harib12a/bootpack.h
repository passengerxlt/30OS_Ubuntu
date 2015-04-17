/* asmhead.asm */
struct BOOTINFO { /* 0x0ff0-0x0fff */
  char cyls;  /* 启动区读硬盘读到何处为止*/
  char leds;  /* 启动时键盘LED的状态*/
  char vmode; /* 显卡模式为多少为彩色*/
  char reserve;
  short scrnx, scrny; /* 画面分辨率 */
  char *vram;
};
#define ADR_BOOTINFO 0x00000ff0

/* asmfunc.asm  */
void _io_hlt(void);
void _write_mem8(int addr, int data);
void io_cli(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);
void asm_inthandler20(void);
void asm_inthandler21(void);
void asm_inthandler27(void);
void asm_inthandler2c(void);
int load_cr0(void);
void store_cr0(int cr0);
void memcpy32(char *d, char *s, int size_32);
void taskswitch4(void);

/* graphic.c  */
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

void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
void init_screen8(char *vram, int xsize, int ysize);

void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);
void putfont8(char *vram, int xszie, int x, int y, char c, char *font);
void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s);
void init_mouse_cursor8(char *mouse, char bc);
void putblock8_8(char *vram, int vxsize, int pxsize,
		 int pysize, int px0, int py0, char *buf, int bxsize);

/* dsctbl.c  */
#define ADR_IDT           0x0026f800
#define LIMIT_IDT         0x000007ff
#define ADR_GDT           0x00270000
#define LIMIT_GDT         0x0000ffff
#define ADR_BOTPAK        0x00280000
#define LIMIT_BOTPAK      0x0007ffff
#define AR_DATA32_RW      0x4092
#define AR_CODE32_ER      0x409a
//#define AR_CODE32_ER      0x4f9a
#define AR_INTGATE32	  0x008e
#define AR_TSS32		 	0x0089

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

void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar);
void init_gdtidt(void);

/* int.c */
#define PIC0_ICW1          0x0020
#define PIC0_OCW2          0x0020
#define PIC0_IMR           0x0021
#define PIC0_ICW2          0x0021
#define PIC0_ICW3          0x0021
#define PIC0_ICW4          0x0021
#define PIC1_ICW1          0x00a0
#define PIC1_OCW2          0x00a0
#define PIC1_IMR           0x00a1
#define PIC1_ICW2          0x00a1
#define PIC1_ICW3          0x00a1
#define PIC1_ICW4          0x00a1
void init_pic(void);
void test_int(int index);

/* sprintf.c */
int my_sprintf(char *buf, const char *fmt, ...);

/* fifo.c */
struct FIFO32 {
	int *buf;
	int p, q, size, free, flags;
};

void fifo32_init(struct FIFO32 *fifo, int size, int *buf);
int fifo32_put(struct FIFO32 *fifo, int data);
int fifo32_get(struct FIFO32 *fifo);
int fifo32_status(struct FIFO32 *fifo);

#define PORT_KEYDAT          0x0060
#define PORT_KEYSTA          0x0064
#define PORT_KEYCMD          0x0064
#define KEYSTA_SEND_NOTREADY 0x02
#define KEYCMD_WRITE_MODE    0x60
#define KBC_MODE             0x47

#define KEYCMD_SENDTO_MOUSE  0xd4
#define MOUSECMD_ENABLE      0xf4
/* keyboard.c*/
void init_keyboard(struct FIFO32 *fifo, int data0);
void inthandler21(int *esp);

/* mouse.c*/
struct MOUSE_DEC {
  unsigned char buf[3], phase;
  int x, y, btn;
};
void enable_mouse(struct FIFO32 *fifo, int data0, struct MOUSE_DEC *mdec);
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat);
void inthandler2c(int *esp);

/* memory.c */
#define MEMMAN_FREES     4096

struct FREEINFO {
  unsigned int addr, size;
};

struct MEMMAN {
  int frees, maxfrees, lostsize, losts;
  struct FREEINFO free[MEMMAN_FREES];
};
unsigned int memtest_sub(unsigned int start, unsigned int end);
void memman_init(struct MEMMAN *man);
unsigned int memman_total(struct MEMMAN *man);
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size);
int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size);

/* sheet.c*/
struct SHEET {
  unsigned char *buf;
  int bxsize, bysize, vx0, vy0, col_inv, height, flags;
  struct SHTCTL *ctl;
};

#define MAX_SHEETS     256

struct SHTCTL {
  unsigned char *vram, *map;
  int xsize, ysize, top;
  struct SHEET *sheets[MAX_SHEETS];
  struct SHEET sheets0[MAX_SHEETS];
};
struct SHTCTL * shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize);
struct SHEET * sheet_alloc(struct SHTCTL *ctl);
void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int bxsize, int bysize, int col_inv);
void sheet_updown(struct SHEET *sht, int height);
void sheet_slide(struct SHEET *sht, int vx0, int vy0);
void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1);

/* echo.c */
void echo(struct BOOTINFO *binfo, unsigned char *s);

/*timer.h*/
#define MAX_TIMER	500
struct TIMER {
	struct TIMER *next;
	unsigned int timeout, flags;
	struct FIFO32 *fifo;
	int data;
};

struct TIMERCTL {
	unsigned int count, next;
	struct TIMER *t0;
	struct TIMER timers0[MAX_TIMER];
};

void init_pit(void);
void inthandler20(int *esp);
struct TIMER *timer_alloc(void);
void timer_free(struct TIMER *timer);
void timer_init(struct TIMER *timer, struct FIFO32 *fifo, int data);
void timer_settime(struct TIMER *timer, unsigned int timeout);

//
struct TSS32 {
	int backlink, esp0, ss0, esp1, ss1, esp2, ssp2, cr3;
	int eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
	int es, cs, ss, ds, fs, gs;
	int ldtr, iomap;
};
