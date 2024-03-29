#ifndef UTILS_H
#define UTILS_H

#include "stdint.h"

void *simple_malloc(int size);
int strcmp(char *str1, char *str2);
int strncmp(const char *s1, const char *s2, int n);
int memcmp(void *s1, void *s2, int n);
void *memcpy(void *dest, const void *src, long long len);
int strlen(char *str);
char *strcpy(char *des, const char *src);
char *strncpy(char *des, const char *src, int n);
char *strclear(char *s);
void swap(int *a, int *b);
int pow(int base, int exponent);
int hex2dec(char *s, int n);
int atoh(char *str);
int allOne(long long n);
int log(int n);
void delay(int sec);
char *strtok(char* string_org, const char* demial);
char *strcat(char *dst, char *src);

#endif