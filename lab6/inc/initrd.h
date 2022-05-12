#ifndef INITRD__H
#define INITRD__H

void parse_cpio_name();
void parse_cpio_file();
char *get_cpio_file(char *filename);
int get_cpio_file_len(char *filename);
void get_initrd_start(unsigned int addr);

#endif