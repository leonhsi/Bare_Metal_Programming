#include "uart.h"

char* malloc_addr = (char*)0x6000000;

void* simple_malloc(int size){
	printf("\n%x\n", malloc_addr);
	void *ptr = malloc_addr;
	malloc_addr += size;

	printf("\n%x\n", malloc_addr);

	return ptr;
}

int strcmp(char *str1, char *str2)
{
	while( (*str1 != '\0' && *str2 != '\0') && *str1 == *str2 )
	{
		str1++;
		str2++;
	}

	if(*str1 == *str2)
		return 0;
	else
		return *str1 - *str2;
}

int memcmp(void *s1, void *s2, int n){  //n : bytes
	unsigned char *a = s1, *b = s2;
	while(n-->0){
		if(*a!=*b){
			return *a-*b;
		}
		a++;
		b++;
	}

	return 0; //same
}

int strlen(char *str){
	int len;
	for(len = 0; *str; str++) len++;

	return len;
}
