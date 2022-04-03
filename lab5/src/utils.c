#include "uart.h"
#include "utils.h"

char* malloc_addr_start = (char*)0x6000000;
char* malloc_addr_end =   (char*)0x7000000;
char* malloc_addr = 	  (char*)0x6000000;

void* simple_malloc(int size){
	//printf("\n%x\n", malloc_addr);
	
	if(malloc_addr + size > malloc_addr_end){
		printf("allocate size out of range\n");
		return "";
	}
	void *ptr = malloc_addr;
	malloc_addr += size;

	//printf("\n%x\n", malloc_addr);

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

int strncmp(const char *s1, const char *s2, int n){
	unsigned char u1, u2;
	while(n--){
		u1 = (unsigned char)(*s1++);
		u2 = (unsigned char)(*s2++);
		if(u1 != u2){
			return u1 - u2;
	
		}
		if(u1 == '\0'){
			return 0;
		}
	}

	return 0;
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

void *memcpy(void *dest, const void *src, long long len){
	char *d = dest;
	const char *s = src;
	while(len--){
		*d++ = *s++;
	}
	return dest;
}

int strlen(char *str){
	int len;
	for(len = 0; *str; str++) len++;

	return len;
}

char *strcpy(char *des, char *src){
	char *start = des;
	while(*src != '\0'){
		*des = *src;
		des++;
		src++;
	}

	*des = '\0';
	return start;
}

char *strncpy(char *des, const char *src, int n){
	char *ptr = des;

	while(*src && n--){
		*des = *src;
		des++;
		src++;
	}

	*des = '\0';

	return ptr;
}

char *strclear(char *s){
	char *ptr = s;
	int len = strlen(s);

	for(int i=0; i<len; i++){
		*s = '\0';
		s++;
	}

	*s = '\0';
	return ptr;
}

void swap(int *a, int *b){
	int temp = *a;
	*a = *b;
	*b = temp;
}

int pow(int base, int exponent){
	int res = 1;

	for(int i=0; i<exponent; i++){
		res *= base;
	}

	return res;
}

int hex2dec(char *s, int n){
	int res = 0;
	while(n--){
		res <<= 4;
		if(*s >= 'A' && *s <= 'F'){
			res += 10 + *s++ - 'A';
		}
		else{
			res += *s++ - '0';
		}
	}
	return res;
}

int atoh(char *str){
	int res = 0;

	for(int i=0; str[i] != '\0'; i++){
		res = res * 16 + str[i] - '0';
	}	

	return res;
}

//check if all bits are 1
int allOne(long long n){
	if(n == 0){
		return 0;
	}

	int cnt = sizeof(long long);
	while(cnt--){
		if((n & 1) == 0){
			return 0;
		}

		n >>= 1;
	}

	return 1;
}

int log(int n){
	int res = 0;
	while(n >1){
		n >>= 1;
		res++;
	}

	return res;
}

void delay(int sec){
	while(sec--){
		asm volatile ("nop");
	}
}
