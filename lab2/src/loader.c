#include "uart.h"
#include "loader.h"

#define OLD_KERNEL_ADDRESS 0x60000
#define NEW_KERNEL_ADDRESS 0x80000

void copy_old_kernel()
{
	extern volatile unsigned char _start;
	extern volatile unsigned char _end;

	char *start = (char*)&_start;
	char *end = (char*)&_end;

	char *mem_addr = (char *)OLD_KERNEL_ADDRESS;

	printf("\nmove to : %x\n", mem_addr);
	printf("start from : %x\n", start);
	printf("end at : %x\n", end);

	while(start <= end)
	{
		*mem_addr = *start;
		start++;
		mem_addr++;
	}
	
	printf("finish at : %x\n", mem_addr);
	printf("from %x\n", start);

	void (*func_ptr)() = (void *)(OLD_KERNEL_ADDRESS + load_new_kernel - NEW_KERNEL_ADDRESS);
	(func_ptr)();
}

void load_new_kernel()
{

	//printf("\naaaaaa\n");

	int length = 0;
	char c;
	while((c = uart_getc_nr()) != '\n'){
		int tmp = c - '0';
		printf("%d ", tmp);
		length = length * 10 + tmp;
	}

	char *mem_addr = (char *)NEW_KERNEL_ADDRESS;
	int cnt = 0;

	printf("\nsize : %x\n", length);

	while(cnt < length)
	{
		*(mem_addr + cnt) = uart_getc_nr();
		cnt++;
	}

	printf("finish %d\n", cnt);

	void (*func_ptr)() = (void *)NEW_KERNEL_ADDRESS;
	(func_ptr)();
}