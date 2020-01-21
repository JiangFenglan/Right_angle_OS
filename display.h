/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _DISPLAY_H
#define _DISPLAY_H

/* Includes ------------------------------------------------------------------*/
#include "type.h"
/* Exported types ------------------------------------------------------------*/

void Gui_background(void);
void init_mouse_cursor8(char *mouse, char bc);
void putblock8_8(char *vram, int vxsize, int pxsize,int pysize, int px0, int py0, char *buf, int bxsize);
void init_palette (void); //初始化调色板
void set_palette( int start ,int end ,u8 *rgb);

typedef struct
{
	u8 cyls,leds,vmode,reserve;
	u16	scrnx,scrny;
	char *vram;
}Bootinfor_t;


#define infor ((Bootinfor_t*)0x0ff0)

int my,mx;

char mcursor[256];

/* Define ---------------------------------------------------------------------*/
#define black     0
#define red       1
#define grean     2
#define yellow    3
#define blue    	4
#define purple    5
#define light_blue    6
#define white    			7
#define grey    			8
#define dull_red     	9
#define dull_grean    10
#define dull_yellow   11
#define dull_blue     12
#define dull_purple   13
#define blue_light    14
#define dull_grey     15

#define ADR_BOOT      0x0ff0

#define LINE_LAST 32

#define BACKGROUND_COLOUR	blue_light
#define MENU_BORDE	185
/* Exported constants --------------------------------------------------------*/


void io_cli(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);
void putfonts8_str(char *vram , int xsize, int x, int y, char color ,char *str);
void putfont8 (char *vram , int xsize, int x, int y, char color ,char *font );
void putfonts8_str(char *vram , int xsize, int x, int y, char color ,char *str);



/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

void init_palette (void) //初始化调色板
{
	//先定义颜色
	static u8 table_rgb[16*3]={
	
	  0x00, 0x00, 0x00,	/*  0:black */
		0xff, 0x00, 0x00,	/*  1:red   */
		0x00, 0xff, 0x00,	/*  2:grean */
		0xff, 0xff, 0x00,	/*  3:yellow */
		0x00, 0x00, 0xff,	/*  4:blue*/
		0xff, 0x00, 0xff,	/*  5:purple*/
		0x00, 0xff, 0xff,	/*  6:light blue */
		0xff, 0xff, 0xff,	/*  7:white*/
		0xc6, 0xc6, 0xc6,	/*  8:grey */
		0x84, 0x00, 0x00,	/*  9:dull_red */
		0x00, 0x84, 0x00,	/* 10:dull_grean */
		0x84, 0x84, 0x00,	/* 11:dull_yellow */
		0x00, 0x00, 0x84,	/* 12:dull_blue */
		0x84, 0x00, 0x84,	/* 13:dull_purple */
		0x00, 0x84, 0x84,	/* 14:blue_light */
		0x84, 0x84, 0x84	/* 15:dull_grey*/	
	};
	set_palette( 0 , 15 ,table_rgb);//设置调色板
} //ok

void set_palette( int start ,int end ,u8 *rgb)
{
	int i,eflags;
	eflags=io_load_eflags(); //记录中断许可标志的值
	io_cli();                //将标志置0
	io_out8(0x03c8 , start); //要想设置颜色，首先得先向这个寄存器写入色号，也就是从0-15个号码
	for(i=start; i<=end ;i++) //开始写入rgb的数值用于调制颜色
	{
		io_out8(0x03c9,rgb[0]/4);
		io_out8(0x03c9,rgb[1]/4);
		io_out8(0x03c9,rgb[2]/4);	
		rgb+=3;
	}
	io_store_eflags(eflags);
}

void putfont8 (char *vram , int xsize, int x, int y, char color ,char *font )
{
	u8 i;
	u8 *p, data;
	for(i=0; i<16; i++)
	{
		p=vram+(y+i)*xsize+x;
		data = font[i];
		if((data & 0x80) !=0)	{p[0]= color ;}
		if((data & 0x40) !=0)	{p[1]= color ;}
		if((data & 0x20) !=0)	{p[2]= color ;}	
		if((data & 0x10) !=0)	{p[3]= color ;}
			
		if((data & 0x08) !=0)	{p[4]= color ;}	
		if((data & 0x04) !=0)	{p[5]= color ;}
		if((data & 0x02) !=0)	{p[6]= color ;}	
		if((data & 0x01) !=0)	{p[7]= color ;}		
	}
	
}

void putfonts8_str(char *vram , int xsize, int x, int y, char color ,char *str)
{
	extern char hankaku[4096];
	for( ; *str!=0x00;str++)
	{
		putfont8(vram,xsize,x,y,color,hankaku+*str*16);
		x+=8;
		if(x>=xsize)
			{
				y+=16;
			}
	}
}


void printfrectangle(u8 *vram, int xsize, u8 color, int x_start, int y_start, int x_end, int y_end)
{
	int x,y;
	for(y=y_start; y<y_end; y++)
	{
		for(x=x_start; x<x_end; x++)
			vram[y*xsize + x ]=color;
	}
}

void init_mouse_cursor8(char *mouse, char bc)
/* 16*16的鼠标 */
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

	for (y = 0; y < 16; y++) 
	{
		for (x = 0; x < 16; x++) {
			if (cursor[y][x] == '*') {
				mouse[y * 16 + x] = black;
			}
			if (cursor[y][x] == 'O') {
				mouse[y * 16 + x] = white;
			}
			if (cursor[y][x] == '.') {
				mouse[y * 16 + x] = bc;
			}
		}
	}
}

void putblock8_8(char *vram, int vxsize, int pxsize,int pysize, int px0, int py0, char *buf, int bxsize)
{
	int x, y;
	for (y = 0; y < pysize; y++) {
		for (x = 0; x < pxsize; x++) {
			vram[(py0 + y) * vxsize + (px0 + x)] = buf[y * bxsize + x];
		}
	}
}
void Gui_background(void)
{
	
	char *vram;
	int xsize, ysize;
	
	Bootinfor_t *binfo;

	init_palette();
	binfo = ((Bootinfor_t*) 0x0ff0);
	
	xsize = (*binfo).scrnx;
	ysize = (*binfo).scrny;
	vram  = (*binfo).vram;
	
	printfrectangle(vram,xsize,blue_light,0,0,xsize,MENU_BORDE); //zhujiemian
	printfrectangle(vram,xsize,dull_blue,0,MENU_BORDE+1,xsize,MENU_BORDE+2); //meun
	printfrectangle(vram,xsize,dull_grey,0,MENU_BORDE+2,xsize,ysize);
	
	printfrectangle(vram,xsize,dull_yellow,4,190,20,ysize-4);
	printfrectangle(vram,xsize,yellow,2,MENU_BORDE+3,22,ysize-2);
	
	//putfonts8_str( vram ,xsize, 8, 8 , white , " Welcome to RightAngle OS");
}

#endif /*_DISPLAY_H */

