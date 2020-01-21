
#include"display.h"
#include"type.h"



void io_hlt(void);
void write_mem8(int addr, int data);
int io_load_eflags(void);
void load_gdtr(int limit, int addr );
void io_store_eflags(	int eflags);
void io_out8(int port, int data);
void io_out16(int port, int data);
void io_out16(int port, int data);
int io_in8(int port);
int io_in16(int port);
int io_in32(int port);
void io_cli(void);
void io_sti(void);
void io_stihlt(void);
void load_idtr(int limit, int addr);
void asm_inthandler21(void);
void asm_inthandler27(void);
void asm_inthandler2c(void);


