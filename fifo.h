#include"type.h"

typedef struct
{
	u8 *buf;
	u16 next_r, next_w, size, free, flags;
} FIFO8;


#define FLAGS_OVERRUN		0x0001


u8 fifo8_get(FIFO8 *fifo);
u8 fifo8_put(FIFO8 *fifo, u8 data);
u8 fifo8_get(FIFO8 *fifo);
int fifo8_status(FIFO8 *fifo);

void fifo_init( FIFO8 *fifo, u8 *buf, u16 size)
{
	fifo->buf=buf;
	fifo->next_r=0;
	fifo->next_w=0;
	fifo->size=size;
	fifo->free=size;
	fifo->flags=0;
}

u8 fifo8_put(FIFO8 *fifo, u8 data)//保存数据
{
	if (fifo->free == 0) //溢出
	{
		fifo->flags |= FLAGS_OVERRUN;
		return -1;
	}
	fifo->buf[fifo->next_w] = data;
	fifo->next_w++;
	if (fifo->next_w== fifo->size) 
	{
		fifo->next_w = 0;
	}
	fifo->free--;
	return 0;
}

u8 fifo8_get(FIFO8 *fifo)
{
	u8 data;
	if(fifo->free == fifo->size)
	{
		return -1; // if buf is blonk ,return -1
	}
	data=fifo->buf[fifo->next_r];
	fifo->next_r++;
	if (fifo->next_r == fifo->size)
	{
		fifo->next_r=0; //  de overflow
	}
	fifo->free++; //free space
	return data;
}
int fifo8_status(FIFO8 *fifo)
{
	return fifo->size - fifo->free;
}

