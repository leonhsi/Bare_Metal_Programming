#ifndef INITRD_H
#define INITRD_H

extern unsigned long cpio_addr;

typedef struct{
	char	   c_magic[6];
	char	   c_ino[8];
	char	   c_mode[8];
	char	   c_uid[8];
	char	   c_gid[8];
	char	   c_nlink[8];
	char	   c_mtime[8];
	char	   c_filesize[8];
	char	   c_devmajor[8];
	char	   c_devminor[8];
	char	   c_rdevmajor[8];
	char	   c_rdevminor[8];
	char	   c_namesize[8];
	char	   c_check[8];
}__attribute__((packed)) cpio_t;

void parse_cpio_name();
void parse_cpio_file();
char *get_cpio_file(char *filename);
int get_cpio_file_len(char *filename);
void get_initrd_start(char *prop_name, unsigned int addr);

#endif