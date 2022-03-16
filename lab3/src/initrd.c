#include "uart.h"
#include "utils.h"

unsigned long cpio_addr;

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

void parse_cpio_name(){

	printf("\n");

	char *addr = (char *)cpio_addr;

	while(!memcmp(addr, "070701", 6) && memcmp(addr+sizeof(cpio_t), "TRAILER!!!", 9)){
		cpio_t *header = (cpio_t *)addr;
		int ns = hex2dec(header->c_namesize, 8);
		int fs = hex2dec(header->c_filesize, 8);

		//align
		if((fs%4) != 0){
			fs += 4 - (fs%4);
		}
		if( ((sizeof(cpio_t) + ns) % 4) != 0){
			ns += 4 - (sizeof(cpio_t)+ns)%4;
		}

		printf("%s\n", addr+sizeof(cpio_t));
		addr += (sizeof(cpio_t) + ns + fs);
	}
}

void parse_cpio_file(){

	printf("\nFilename: ");

	char c;
	char filename[100];
	int len = 0;

	while((c = uart_getc()) != '\n'){
		uart_send(c);
		filename[len] = c;
		len++;
	}

	char *addr = (char *)cpio_addr;

	while(!memcmp(addr, "070701", 6) && memcmp(addr+sizeof(cpio_t), "TRAILER!!!", 9)){

		cpio_t *header = (cpio_t *)addr;
		int ns = hex2dec(header->c_namesize, 8);
		int fs = hex2dec(header->c_filesize, 8);

		int file_len;
		if((ns-1) > len) file_len = (ns-1);
		else file_len = len;

		//align
		if((fs%4) != 0){
			fs += 4 - (fs%4);
		}
		if(((sizeof(cpio_t) + ns) % 4) != 0){
			ns += 4 - (sizeof(cpio_t)+ns)%4;
		}

		if(!memcmp(addr+sizeof(cpio_t), filename, file_len)){
			printf("\n%s\n", addr+sizeof(cpio_t)+ns);

			return;
		}

		addr += (sizeof(cpio_t) + ns + fs);
	}

	printf("\nFile not found\n");

}

void get_cpio_addr(unsigned int addr){
	cpio_addr = addr;
}
