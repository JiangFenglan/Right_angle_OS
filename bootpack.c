#include<stdio.h>
#include"type.h"
#include"system.h"
#include"interrupt.h"

void HariMain(void)
{
	//u16 *addr;
	mx = (infor->scrnx - 16) / 2;
	my = (infor->scrny - 28 - 16) / 2;

	init_GDT_IDT();
	init_pic();
	io_sti(); 
	Gui_background();
	init_mouse_cursor8(mcursor, blue_light);
	putblock8_8(infor->vram, infor->scrnx, 16, 16, mx, my, mcursor, 16);
	
	//sprintf(addr, "scrnx = %d",infor->scrnx);
	//putfonts8_str( infor->vram ,infor->scrnx, 8, 24 , white ,addr);
	io_out8(PIC0_IMR, 0xf9); /*(11111001) */
	io_out8(PIC1_IMR, 0xef); /*(11101111) */
	//putfonts8_str( infor->vram ,infor->scrnx, 8, 8 , white , " Welcome to RightAngle OS");
	while(1){
		io_cli();
		//putfonts8_str( infor->vram ,infor->scrnx, 8, 23 , white , " Welcome to RightAngle BS");
		key_hander();
		mouse_decode();
		//mouse_hander();
	}
}





