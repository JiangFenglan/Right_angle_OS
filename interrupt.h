#include <stdbool.h>
#include "system.h"
#include "fifo.h"
#include"display.h"

#define PIC0_ICW1		0x0020
#define PIC0_OCW2		0x0020
#define PIC0_IMR		0x0021
#define PIC0_ICW2		0x0021
#define PIC0_ICW3		0x0021
#define PIC0_ICW4		0x0021
#define PIC1_ICW1		0x00a0
#define PIC1_OCW2		0x00a0
#define PIC1_IMR		0x00a1
#define PIC1_ICW2		0x00a1
#define PIC1_ICW3		0x00a1
#define PIC1_ICW4		0x00a1

#define ADR_IDT			0x0026f800
#define LIMIT_IDT		0x000007ff
#define ADR_GDT			0x00270000
#define LIMIT_GDT		0x0000ffff
#define ADR_BOTPAK		0x00280000
#define LIMIT_BOTPAK	0x0007ffff
#define AR_DATA32_RW	0x4092
#define AR_CODE32_ER	0x409a
#define AR_INTGATE32	0x008e

#define PORT_KEYDATA	0x0060
#define PORT_KEYSTA		0x0064
#define PORT_KEYCMD		0x0064
#define KEYSTA_SEND_NOTREADY 	0x02
#define KEYCMD_WRITE_MODE		0x60
#define KBC_MODE		0x47
#define KEYCMD_SENDTO_MOUSE 0xd4
#define MOUSECMD_ENABLE	 0xf4



Bootinfor_t *binfo=(Bootinfor_t *) ADR_BOOT;
typedef struct  {
	short offset_low, selector;
	char dw_count, access_right;
	short offset_high;
}GATE_DESCRIPTOR;

typedef struct  {
	short limit_low, base_low;
	char base_mid, access_right;
	char limit_high, base_high;
}SEGMENT_DESCRIPTOR;


typedef struct  {
	u8 data[32]; //keyvalue 缓存区
	u8 next_w,next_r,len;	
}KEYBUF_t;


FIFO8 keyfifo,mousefifo;

u8 key_fifobuf[32];
u8 mouse_fifobuf[128];

typedef struct{
	u8 mouse_phase;
	u8 mouse_dbuf[3];
	s16 lable_x;
	s16 lable_y;
	s16  bnt;	
}mouse_decode_t;
	
mouse_decode_t mouse_d;


void inthandler21(int *esp);
void inthandler27(int *esp);
void inthandler2c(int *esp);
void init_GDT_IDT( void );
void set_segmdesc( SEGMENT_DESCRIPTOR *sd, unsigned int limit, u32 base, int ar);
void set_gatedesc( GATE_DESCRIPTOR *gd, int offset, int selector, int ar);
void enable_mouse(void);
void init_keyboardc (void );
u8 mouse_hander(u8 data);
void mouse_decode(void);


u16 line=0,row=0;
KEYBUF_t keybuf;
void init_pic(void)
{
	io_out8(PIC0_IMR,  0xff  ); /* 中断屏蔽寄存器，屏蔽所有中断 */
	io_out8(PIC1_IMR,  0xff  ); /* 中断屏蔽寄存器，屏蔽所有中断 */

	io_out8(PIC0_ICW1, 0x11  ); /* 边沿触发*/
	io_out8(PIC0_ICW2, 0x20  ); /* IRQ0-7偼丄INT20-27偱庴偗傞 */
	io_out8(PIC0_ICW3, 1 << 2); /* PIC1偼IRQ2偵偰愙懕 */
	io_out8(PIC0_ICW4, 0x01  ); /* 僲儞僶僢僼傽儌乕僪 */

	io_out8(PIC1_ICW1, 0x11  ); /* 僄僢僕僩儕僈儌乕僪 */
	io_out8(PIC1_ICW2, 0x28  ); /* IRQ8-15偼丄INT28-2f偱庴偗傞 */
	io_out8(PIC1_ICW3, 2     ); /* PIC1由IRQ2*/
	io_out8(PIC1_ICW4, 0x01  ); /* 无缓冲区模式 */

	io_out8(PIC0_IMR,  0xfb  ); /* 11111011 PIC1除外的所有中断*/
	io_out8(PIC1_IMR,  0xff  ); /* 11111111 禁止所有中断*/

	fifo_init(&keyfifo,key_fifobuf,32);
	fifo_init(&mousefifo,mouse_fifobuf,128);
	
	init_keyboardc();
	enable_mouse();
	return;
}



/*************************************************************************/
/*************************************************************************/
#if 0
void mouse_decode(u8 data)
{
	u8 str[4];
	if(mouse_d.mouse_phase==1)
	{
		mouse_d.mouse_dbuf[2]=data;
		sprintf(str,"%02X %02X %02X",mouse_d.mouse_dbuf[0],mouse_d.mouse_dbuf[1],mouse_d.mouse_dbuf[2]);
		printfrectangle(binfo->vram,binfo->scrnx,black,8,8,8+16*4,8+16);
		putfonts8_str(binfo->vram,binfo->scrnx,8,8,white,str);	
	}
	else if(mouse_d.mouse_phase == 2)
	{
		mouse_d.mouse_dbuf[0]=data;
	}
	else if(mouse_d.mouse_phase == 3)
	{
		mouse_d.mouse_dbuf[1]=data;
	}
	

}
#endif
u8 mouse_hander(u8 data)
{

	if (mouse_d.mouse_phase==0)
	{
		if(data == 0xfa)
		{
			mouse_d.mouse_phase=1;
		}
		return 0;
	}
	else
	{
		if(mouse_d.mouse_phase==3)
		{
			mouse_d.mouse_dbuf[2]=data;
			mouse_d.mouse_phase=1;

			mouse_d.bnt=mouse_d.mouse_dbuf[0] & 0x07;
			mouse_d.lable_x=mouse_d.mouse_dbuf[1];
			mouse_d.lable_y=mouse_d.mouse_dbuf[2];

			if((mouse_d.mouse_dbuf[0] & 0x10))
			{
				mouse_d.lable_x |= 0xff00;
			}
			
			if((mouse_d.mouse_dbuf[0] & 0x20))
			{
				mouse_d.lable_y |= 0xff00;
			}

			mouse_d.lable_y = -mouse_d.lable_y;
			return 1;
		
		}
			else if(mouse_d.mouse_phase == 1)
			{
				if((data & 0xc8)==0x08)
				{
					mouse_d.mouse_dbuf[0]=data;
					mouse_d.mouse_phase=2;
				}	
				return 0;
			}
			else if(mouse_d.mouse_phase == 2)
			{
				mouse_d.mouse_dbuf[1]=data;
				mouse_d.mouse_phase=3;
				return 0;			}
		
	}
	
}


void mouse_decode (void)
{
	u8 str[10];
	u8 str1[8];
	u8 data;
	if(fifo8_status(&mousefifo)==0)
	{
		io_stihlt();
	}else
	{
		io_sti();
		data=fifo8_get(&mousefifo);
		if (mouse_hander(data))
		{
			if(mouse_d.bnt & 0x01)
			{
				str[1]='L';
			}
			if(mouse_d.bnt & 0x02)
			{
				str[3]='R';
			}
			if(mouse_d.bnt & 0x04)
			{
				str[2]='C';
			}
			
			/****鼠标指针移动*****/
			printfrectangle(binfo->vram,binfo->scrnx,BACKGROUND_COLOUR,mx,my,mx+16,my+16);
			mx+=mouse_d.lable_x;
			my+=mouse_d.lable_y;

			if(mx<0)
			{
				mx=0;
			}
			if(my<0)
			{
				my=0;
			}
			if(mx>binfo->scrnx-16)
			{
				mx=binfo->scrnx-16;
			}
			if(my>MENU_BORDE-16)
			{
				my=MENU_BORDE-16;
			}
			
			sprintf(str1,"(%3d,%3d)",mx,my);			
			printfrectangle(binfo->vram,binfo->scrnx,BACKGROUND_COLOUR,8,8,8+16*8,8+16);
			putfonts8_str(binfo->vram,binfo->scrnx,8,8,white,str1);
			putblock8_8(infor->vram, infor->scrnx, 16, 16, mx, my, mcursor, 16);			
		}
	}
	
		
}
void key_hander(void)
{
	u8 str[4];
	u8 data_count=0;	
	u8 data;
	if(fifo8_status(&keyfifo)==0)
	{
		io_stihlt();
	}else
	{

		data=fifo8_get(&keyfifo);
		io_sti();
		
		sprintf(str,"%02X",data);
		io_sti();
		printfrectangle(binfo->vram,binfo->scrnx,BACKGROUND_COLOUR,8,24,8+16,24+16);
		putfonts8_str(binfo->vram,binfo->scrnx,8,24,white,str);
		
	}

	
}

void wait_KBC_sendready (void) //等待键盘控制电路初始化完成，鼠标键盘都是一个电路
{
	while(1)
	{
		if ((io_in8(PORT_KEYSTA)& KEYSTA_SEND_NOTREADY)  == 0)
		{
			break;
		}
	}
}

void init_keyboardc (void )
{
	//初始化键盘控制电路
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD,KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDATA,KBC_MODE);	
}

void enable_mouse(void)
{
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD,KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDATA,MOUSECMD_ENABLE);
}

void inthandler21 (int *esp) //keybroad
{	
	u8 data;
	io_out8(PIC0_OCW2,0x61);//clear the interrupt 1+0x60
	data=io_in8(PORT_KEYDATA);
	fifo8_put(&keyfifo,data);
	return;
}

void inthandler2c(int *esp)
{
	u8 data;
	io_out8(PIC1_OCW2,0x64);
	io_out8(PIC0_OCW2,0x62);
	data=io_in8(PORT_KEYDATA);
	fifo8_put(&mousefifo,data);
	return;
}



void inthandler27(int *esp)

{
	io_out8(PIC0_OCW2, 0x67); /* IRQ-07*/
	return;
}

void init_GDT_IDT( void )
{
	SEGMENT_DESCRIPTOR *gdt=(SEGMENT_DESCRIPTOR *) 0x00270000;
	GATE_DESCRIPTOR *idt=(GATE_DESCRIPTOR *) 0x0026f800;
	
	u16 i;
	// GDT 初始化
	for (i=0;i<=LIMIT_GDT/8; i++)
	{
		set_segmdesc(gdt+i,0,0,0);
	}
	set_segmdesc(gdt + 1, 0xffffffff,   0x00000000, AR_DATA32_RW);
	set_segmdesc(gdt + 2, LIMIT_BOTPAK, ADR_BOTPAK, AR_CODE32_ER);
	load_gdtr(LIMIT_GDT, ADR_GDT);
	
	//IDT 初始化
	for (i=0;i<LIMIT_IDT/8; i++)
	{
		set_gatedesc(idt+i,0,0,0);
	}
	load_idtr(LIMIT_IDT, ADR_IDT);

	set_gatedesc(idt+0x21,(int)asm_inthandler21,2*8,AR_INTGATE32);
	set_gatedesc(idt+0x2c,(int)asm_inthandler2c,2*8,AR_INTGATE32);
	set_gatedesc(idt+0x27,(int)asm_inthandler27,2*8,AR_INTGATE32);
	
	
}


void set_segmdesc( SEGMENT_DESCRIPTOR *sd, unsigned int limit, u32 base, int ar)
{
	if (limit > 0xfffff) {
		ar |= 0x8000; /* G_bit = 1 */
		limit /= 0x1000;
	}
	sd->limit_low    = limit & 0xffff;
	sd->base_low     = base & 0xffff;
	sd->base_mid     = (base >> 16) & 0xff;
	sd->access_right = ar & 0xff;
	sd->limit_high   = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0);
	sd->base_high    = (base >> 24) & 0xff;
}

void set_gatedesc( GATE_DESCRIPTOR *gd, int offset, int selector, int ar)
{
	gd->offset_low   = offset & 0xffff;
	gd->selector     = selector;
	gd->dw_count     = (ar >> 8) & 0xff;
	gd->access_right = ar & 0xff;
	gd->offset_high  = (offset >> 16) & 0xffff;
}



