#include "uart.h"

char* malloc_addr = (char*)0x6000000;

void* simple_malloc(int size){
	//printf("\n%x\n", malloc_addr);
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
