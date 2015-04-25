#include "bootpack.h"

void fifo32_init(struct FIFO32 *fifo, int size, int *buf, struct TASK *task)
{
	fifo->size = size;
	fifo->buf = buf;
	fifo->free = size;
	fifo->flags = 0;
	fifo->p = 0;
	fifo->q = 0;
	fifo->task = task;
	return;
}

#define FLAGS_OVERRUN    0x0001
int fifo32_put(struct FIFO32 *fifo, int data)
/*向FIFO传送数据并保存*/
{
  if(0 == fifo->free) {
    fifo->flags |= FLAGS_OVERRUN;
    return -1;
  }
  fifo->buf[fifo->p] = data;
  fifo->p++;
  if(fifo->size == fifo->p) {
    fifo->p =0;
  }
  fifo->free--;
	if (0 != fifo->task) {
		if (2 != fifo->task->flags) {
			task_run(fifo->task);
		}
	}
  return 0;
}

int fifo32_get(struct FIFO32 *fifo)
/*从FIFO取得一个数据*/
{
  int data;
  if(fifo->free == fifo->size) {
    return -1;
  }
  data = fifo->buf[fifo->q];
  fifo->q++;
  if(fifo->q == fifo->size) {
    fifo->q = 0;
  }
  fifo->free++;
  return data;
}

int fifo32_status(struct FIFO32 *fifo)
{
  return fifo->size - fifo->free;
}
