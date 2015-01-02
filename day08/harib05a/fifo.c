#include "bootpack.h"

void fifo8_init(struct FIFO8 *fifo, int size, unsigned char *buf)
{
  fifo->size = size;
  fifo->buf = buf;
  fifo->free = size; /*缓冲区的剩余大小*/
  fifo->flags = 0;
  fifo->next_w = 0; /*下一个数据写入位置*/
  fifo->next_r = 0; /*下一个数据读出位置*/
  return;
}

#define FLAGS_OVERRUN    0x0001
int fifo8_put(struct FIFO8 *fifo, unsigned char data)
/*向FIFO传送数据并保存*/
{
  if(0 == fifo->free) {
    fifo->flags |= FLAGS_OVERRUN;
    return -1;
  }
  fifo->buf[fifo->next_w] = data;
  fifo->next_w++;
  if(fifo->size == fifo->next_w) {
    fifo->next_w =0;
  }
  fifo->free--;
  return 0;
}

int fifo8_get(struct FIFO8 *fifo)
/*从FIFO取得一个数据*/
{
  int data;
  if(fifo->free == fifo->size) {
    return -1;
  }
  data = fifo->buf[fifo->next_r];
  fifo->next_r++;
  if(fifo->next_r == fifo->size) {
    fifo->next_r = 0;
  }
  fifo->free++;
  return data;
}

int fifo8_status(struct FIFO8 *fifo)
{
  return fifo->size - fifo->free;
}
