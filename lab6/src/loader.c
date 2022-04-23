#include "uart.h"
#include "loader.h"

//#define OLD_KERNEL_ADDRESS (volatile unsigned char*)0x60000
//#define OLD_KERNEL_ADDRESS 0x60000
//#define NEW_KERNEL_ADDRESS 0x80000
#define OLD_KERNEL_ADDRESS 0xFFFF000000060000
#define NEW_KERNEL_ADDRESS 0xFFFF000000080000

void copy_old_kernel()
{
	extern volatile unsigned char _start;
	extern volatile unsigned char _end;

	char *start = (char*)&_start;
	char *end = (char*)&_end;

	char *mem_addr = (char *)OLD_KERNEL_ADDRESS;

	printf("\n%x\n", mem_addr);
	printf("%x\n", start);
	printf("%x\n", end);

	while(start <= end)
	{
		*mem_addr = *start;
		start++;
		mem_addr++;
	}
	
	printf("%x\n", mem_addr);
	printf("%x\n", start);

	//function pointer
	void (*func_ptr)() = (void *)(OLD_KERNEL_ADDRESS + load_new_kernel - NEW_KERNEL_ADDRESS);
	(*func_ptr)();
}

void load_new_kernel()
{

	printf("\naaaaaa\n");

	int length = 0;
	char c;
	while((c = uart_getc()) != '\n'){
		int tmp = c - '0';
		printf("%d ", tmp);
		length = length * 10 + tmp;
	}

	char *mem_addr = (char *)NEW_KERNEL_ADDRESS;
	int cnt = 0;

	printf("\n%d\n", length);

	while(cnt < length)
	{
		char c = uart_getc();
		*(mem_addr + cnt) = c;
		cnt++;
	}

	printf("%d\n", cnt);

	void (*func_ptr)() = (void *)NEW_KERNEL_ADDRESS;
	(*func_ptr)();
}
