#ifndef UTILS__H
#define UTILS__H

#include "stdint.h"

void *simple_malloc(int size);
int strcmp(char *str1, char *str2);
int strncmp(const char *s1, const char *s2, int n);
int memcmp(void *s1, void *s2, int n);
void *memcpy(void *d, void *s, size_t n);
void *memset(void *d, int v, size_t n);
int strlen(char *str);
char *strcpy(char *des, char *src);
char *strncpy(char *des, const char *src, int n);
char *strclear(char *s);
void swap(int *a, int *b);
int pow(int base, int exponent);
int hex2dec(char *s, int n);
int atoh(char *str);
int allOne(long long n);
int log(int n);
void delay(int sec);

#endif 